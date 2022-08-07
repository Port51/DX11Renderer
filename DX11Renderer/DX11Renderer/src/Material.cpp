#include "pch.h"
#include "Material.h"
#include <typeinfo>
#include "GraphicsDevice.h"
#include "VertexShader.h"
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

	enum MaterialParseState { None, Properties, Pass };

	Material::Material(const GraphicsDevice& gfx, const std::string_view _materialAssetPath)
		: m_materialAssetPath(std::string(_materialAssetPath))
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
			.AppendInstanceDesc<dx::XMFLOAT4>({ "INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 });
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
			if (p.key == "Properties")
			{
				state = MaterialParseState::Properties;
				pPropertySlot = std::make_unique<BindingList>();
			}
			else if (p.key == "Pass")
			{
				state = MaterialParseState::Pass;
				pMaterialPass = std::make_unique<MaterialPass>();
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
					//pPassStep->AddBinding(RasterizerState::Resolve(gfx, RasterizerState::GetCullModeFromMaterialString(p.values[0])))
					//	.SetupRSBinding();
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

					auto pSampler = Sampler::Resolve(gfx);
					pPropertySlot->AddBinding(std::move(pSampler))
						.SetupPSBinding(slotIdx);
				}
			}
			else if (state == MaterialParseState::Pass)
			{
				if (p.key == "VS")
				{
					// Bind vertex shader and input layout
					const auto vertexShaderName = std::move(p.values[0]);
					auto pVertexShader = VertexShader::Resolve(gfx, vertexShaderName.c_str());
					const auto pvsbc = pVertexShader->GetBytecode();
					auto pInputLayout = InputLayout::Resolve(gfx, std::move(m_vertexLayout), vertexShaderName.c_str(), pvsbc);

					pMaterialPass->SetVertexShader(std::move(pVertexShader), std::move(pInputLayout));
				}
				else if (p.key == "PS")
				{
					const auto pixelShaderName = std::move(p.values[0]);
					pMaterialPass->SetPixelShader(PixelShader::Resolve(gfx, pixelShaderName.c_str()));
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

	void Material::SubmitDrawCommands(const Drawable& drawable, const DrawContext& drawContext) const
	{
		// Only submit draw calls for passes contained in draw context
		for (const auto& passName : drawContext.renderPasses)
		{
			if (m_pMaterialPassesByType.find(passName) != m_pMaterialPassesByType.end())
			{
				const auto& pMaterialPass = m_pMaterialPassesByType.at(passName);
				const auto propertySlotIdx = pMaterialPass->GetPropertySlot();
 				const BindingList* const pPropertySlot = (propertySlotIdx != -1) ? m_pPropertySlots[propertySlotIdx].get() : nullptr;
				pMaterialPass->SubmitDrawCommands(drawable, drawContext, pPropertySlot);
			}
		}
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

	std::shared_ptr<Bindable> Material::Resolve(const GraphicsDevice& gfx, const std::string_view assetPath)
	{
		return std::move(Codex::Resolve<Material>(gfx, GenerateUID(assetPath), assetPath));
	}

	std::string Material::GenerateUID(const std::string_view assetPath)
	{
		return typeid(Material).name() + "#"s + std::string(assetPath);
	}

	void Material::AddBindable(std::shared_ptr<Bindable> pBindable)
	{
		m_pBindables.push_back(pBindable);
	}
}