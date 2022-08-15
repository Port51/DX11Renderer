#include "pch.h"
#include "Material.h"
#include <typeinfo>
#include "GraphicsDevice.h"
#include "VertexShader.h"
#include "HullShader.h"
#include "DomainShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Texture.h"
#include "Sampler.h"
#include "SharedCodex.h"
#include "MaterialPass.h"
#include "TransformCbuf.h"
#include "TextParser.h"
#include "Buffer.h"
#include "Binding.h"
#include "ConstantBuffer.h"
#include "NullPixelShader.h"
#include "RenderConstants.h"
#include "DrawContext.h"
#include "RasterizerState.h"
#include "BindingList.h"
#include "RenderConstants.h"
#include "Drawable.h"
#include "DepthStencilState.h"

#include <fstream>
#include <sstream>
#include <assert.h>

namespace gfx
{
	using namespace std::string_literals;

	enum MaterialParseState { None, Global, Properties, Pass };

	Material::Material(const GraphicsDevice& gfx, const std::string_view _materialAssetPath, const bool instancingOn)
		: m_materialAssetPath(std::string(_materialAssetPath)), m_hasTessellation(false)
	{
		// Ref: https://gamedev.stackexchange.com/questions/93105/binding-an-instance-matrix-with-an-inputlayout

		// todo: make this depend on material props
		m_vertexLayout
			.AppendVertexDesc<dx::XMFLOAT3>({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT3>({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT4>({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT4>({ "VERTCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT2>({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }) // last = # instances to draw before moving onto next instance
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_RNG_AND_INDEX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 });
		assert(m_vertexLayout.GetPerVertexStride() % 16 == 0);

		MaterialParseState state = MaterialParseState::None;

		// For unpacking main properties
		// Must be multiple of 16 bytes
		struct PSMaterialConstant
		{
			dx::XMFLOAT3 materialColor = { 0.8f, 0.8f, 0.8f };
			float roughness;
			BOOL normalMappingEnabled = TRUE; // BOOL uses 4 bytes as it's an int, rather than bool
			float specularPower = 30.0f;
			float reflectivity = 0.0f;
			float noiseIntensity = 0.0f;
			//float padding[1];
		} pmc;
		assert(sizeof(PSMaterialConstant) % 16 == 0);

		//AddBindable(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, std::string(assetPath), pmc, 1u));

		// For unpacking material passes
		RenderPassType materialPassRenderType = RenderPassType::Undefined;
		std::unique_ptr<MaterialPass> pMaterialPass;
		std::unique_ptr<BindingList> pPropertySlot;

		TextParser parser(_materialAssetPath);
		TextParser::ParsedKeyValues p;
		while (parser.ReadParsedLine(p))
		{
			// Apply
			if (p.key == "Global")
			{
				state = MaterialParseState::Global;
			}
			else if (p.key == "Properties")
			{
				state = MaterialParseState::Properties;
				pPropertySlot = std::make_unique<BindingList>();
			}
			else if (p.key == "Pass")
			{
				state = MaterialParseState::Pass;
				pMaterialPass = std::make_unique<MaterialPass>();

				pMaterialPass->SetInstanced(instancingOn);
				if (instancingOn)
				{
					pMaterialPass->AddShaderDefine("INSTANCING_ON");
				}
			}
			else if (p.key == "}")
			{
				// End scope
				if (state == MaterialParseState::Properties)
				{
					m_pPropertySlots.emplace_back(std::move(pPropertySlot));
				}
				else if (state == MaterialParseState::Pass)
				{
					// Init cbuffer
					pMaterialPass->AddBinding(std::move(std::make_shared<ConstantBuffer<PSMaterialConstant>>(gfx, D3D11_USAGE_IMMUTABLE, pmc)))
						.SetupPSBinding(RenderSlots::PS_FreeRendererCB + 0u);

					if (pMaterialPass->GetStencil() == nullptr)
						pMaterialPass->SetStencil(DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Normal));

					if (pMaterialPass->IsInstanced()) m_instancedPasses++;
					else m_nonInstancedPasses++;

					m_pMaterialPassesByType.emplace(materialPassRenderType, std::move(pMaterialPass));
				}
			}
			else if (p.key == "Name")
			{
				if (state == MaterialParseState::Pass)
				{
					// Read pass name and map to render pass
					materialPassRenderType = RenderPassConstants::GetRenderPassType(std::move(p.values[0]));
					assert(materialPassRenderType != RenderPassType::Undefined && "Could not resolve render pass from material pass name.");

					pMaterialPass->SetRenderPass(materialPassRenderType);
				}
			}
			else if (p.key == "Cull")
			{
				if (state == MaterialParseState::Pass)
				{
					// Read culling type
					pMaterialPass->AddBinding(RasterizerState::Resolve(gfx, RasterizerState::GetCullModeFromMaterialString(p.values[0]), D3D11_FILL_MODE::D3D11_FILL_SOLID))
						.SetupRSBinding();
				}
			}
			else if (state == MaterialParseState::Global)
			{
				if (p.key == "Tessellation")
				{
					m_hasTessellation = true;
				}
			}
			else if (state == MaterialParseState::Properties)
			{
				if (p.key == "Slot")
				{
					const auto slotIdx = p.ReadInt(0);
				}
				else if (p.key == "Color")
				{
					pmc.materialColor = { p.ReadFloat(0), p.ReadFloat(1), p.ReadFloat(2) };
				}
				else if (p.key == "Roughness")
				{
					pmc.roughness = p.ReadFloat(0);
				}
				else if (p.key == "Reflectivity")
				{
					pmc.reflectivity = p.ReadFloat(0);
				}
				else if (p.key == "NoiseIntensity")
				{
					pmc.noiseIntensity = p.ReadFloat(0);
				}
				else if (p.key == "Texture")
				{
					// Format: Slot, TextureName
					const auto slotIdx = p.ReadInt(0);
					const auto texPath = std::move(p.values[1]);

					pPropertySlot->AddBinding(Texture::Resolve(gfx, texPath))
						.SetupPSBinding(slotIdx);
				}
			}
			else if (state == MaterialParseState::Pass)
			{
				if (p.key == "VS")
				{
					// Bind vertex shader and input layout
					const auto shaderPath = p.values[0].c_str();
					const auto shaderEntry = (p.values.size() >= 2u) ? p.values[1].c_str() : "main";

					auto pVertexShader = VertexShader::Resolve(gfx, shaderPath, shaderEntry, pMaterialPass->GetShaderDefines());
					const auto pvsbc = pVertexShader->GetBytecode();
					auto pInputLayout = InputLayout::Resolve(gfx, std::move(m_vertexLayout), shaderPath, pvsbc);

					pMaterialPass->SetVertexShader(std::move(pVertexShader), std::move(pInputLayout));
				}
				else if (p.key == "HS")
				{
					// Hull shader
					const auto shaderPath = p.values[0].c_str();
					const auto shaderEntry = (p.values.size() >= 2u) ? p.values[1].c_str() : "main";
					pMaterialPass->SetHullShader(HullShader::Resolve(gfx, shaderPath, shaderEntry, pMaterialPass->GetShaderDefines()));
				}
				else if (p.key == "DS")
				{
					// Domain shader
					const auto shaderPath = p.values[0].c_str();
					const auto shaderEntry = (p.values.size() >= 2u) ? p.values[1].c_str() : "main";
					pMaterialPass->SetDomainShader(DomainShader::Resolve(gfx, shaderPath, shaderEntry, pMaterialPass->GetShaderDefines()));
				}
				else if (p.key == "PS")
				{
					// Pixel shader
					const auto shaderPath = p.values[0].c_str();
					const auto shaderEntry = (p.values.size() >= 2u) ? p.values[1].c_str() : "main";
					pMaterialPass->SetPixelShader(PixelShader::Resolve(gfx, shaderPath, shaderEntry, pMaterialPass->GetShaderDefines()));
				}
				else if (p.key == "PropertySlot")
				{
					// Connects to previously defined properties
					const auto slotIdx = p.ReadInt(0);
					pMaterialPass->SetPropertySlot(slotIdx);
				}
				else if (p.key == "RenderQueue")
				{
					pMaterialPass->SetRenderQueue(p.ReadInt(0));
				}
				else if (p.key == "Stencil")
				{
					if (p.values.at(0) == "Write")
						pMaterialPass->SetStencil(DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Write));
					else if (p.values.at(0) == "Mask")
						pMaterialPass->SetStencil(DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Mask));
					else if (p.values.at(0) == "InverseMask")
						pMaterialPass->SetStencil(DepthStencilState::Resolve(gfx, DepthStencilState::Mode::InverseMask));
				}
				else if (p.key == "Define")
				{
					pMaterialPass->AddShaderDefine(p.values.at(0));
				}
			}

		}
		parser.Release();

	}

	void Material::Release()
	{
	}

	void Material::Bind(const GraphicsDevice& gfx, const std::string_view passName)
	{
		// Is this needed?
	}

	void Material::SubmitDrawCommands(GraphicsDevice& gfx, const Drawable& drawable, const DrawContext& drawContext) const
	{
		// Only submit draw calls for passes contained in draw context
		for (const auto& passName : drawContext.renderPasses)
		{
			if (m_pMaterialPassesByType.find(passName) != m_pMaterialPassesByType.end())
			{
				const auto& pMaterialPass = m_pMaterialPassesByType.at(passName);
				const auto propertySlotIdx = pMaterialPass->GetPropertySlot();
 				const BindingList* const pPropertySlot = (propertySlotIdx != -1) ? m_pPropertySlots[propertySlotIdx].get() : nullptr;
				pMaterialPass->SubmitDrawCommands(gfx, drawable, drawContext, pPropertySlot);
			}
		}
	}

	void Material::VerifyInstancing(const bool requireInstancing) const
	{
		if (requireInstancing && m_nonInstancedPasses > 0u) THROW(m_materialAssetPath + std::string(": Non-instanced material pass in an instanced renderer!"));
		else if (!requireInstancing && m_instancedPasses > 0u) THROW(m_materialAssetPath + std::string(": Instanced material pass in a non-instanced renderer!"));
	}

	const u64 Material::GetMaterialCode(const RenderPassType renderPassType) const
	{
		if (m_pMaterialPassesByType.find(renderPassType) != m_pMaterialPassesByType.end())
		{
			return m_pMaterialPassesByType.at(renderPassType)->GetMaterialCode();
		}
		return m_fallbackMaterialCode;
	}

	const VertexLayout& Material::GetVertexLayout() const
	{
		return m_vertexLayout;
	}

	const bool Material::HasTessellation() const
	{
		return m_hasTessellation;
	}

	std::shared_ptr<Bindable> Material::Resolve(const GraphicsDevice& gfx, const std::string_view assetPath)
	{
		return std::move(Resolve(gfx, assetPath, false));
	}

	std::shared_ptr<Bindable> Material::Resolve(const GraphicsDevice& gfx, const std::string_view assetPath, const bool instancingOn)
	{
		return std::move(Codex::Resolve<Material>(gfx, GenerateUID(assetPath, instancingOn), assetPath, instancingOn));
	}

	std::string Material::GenerateUID(const std::string_view assetPath, const bool instancingOn)
	{
		return typeid(Material).name() + "#"s + std::string(assetPath) + std::string((instancingOn) ? "-INSTANCED" : "");
	}

	void Material::AddBindable(std::shared_ptr<Bindable> pBindable)
	{
		m_pBindables.push_back(pBindable);
	}
}