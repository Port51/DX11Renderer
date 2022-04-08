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

#include <fstream>
#include <sstream>

namespace gfx
{
	using namespace std::string_literals;

	enum MaterialParseState { None, Properties, Pass };

	Material::Material(const GraphicsDevice& gfx, std::string_view _materialAssetPath)
		: m_materialAssetPath(std::string(_materialAssetPath))
	{
		// todo: This is very WIP and should be replaced by something entirely different later
		// with most of the parsing logic moved to a different class

		// todo: make this depend on material props
		m_vertexLayout
			.AppendVertexDesc<dx::XMFLOAT3>({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT3>({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT4>({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT2>({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendInstanceDesc<dx::XMFLOAT3>({ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }); // last = # instances to draw before moving onto next instance
		assert(m_vertexLayout.GetPerVertexStride() % 16 == 0);

		dx::XMFLOAT3 colorProp = { 0.8f, 0.8f, 0.8f };
		float roughnessProp = 0.75f;

		MaterialParseState state = MaterialParseState::None;

		// For unpacking main properties
		// Must be multiple of 16 bytes
		struct PSMaterialConstant
		{
			dx::XMFLOAT3 materialColor;
			float roughness;
			BOOL normalMappingEnabled = TRUE; // BOOL uses 4 bytes as it's an int, rather than bool
			float specularPower = 30.0f;
			float padding[2];
		} pmc;

		//AddBindable(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, std::string(assetPath), pmc, 1u));

		// For unpacking material passes
		std::string materialPassName;
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

					m_pPasses.emplace(materialPassName, std::move(pMaterialPass));
				}
			}
			else if (p.key == "Name")
			{
				if (state == MaterialParseState::Pass)
				{
					// Read pass name
					materialPassName = std::move(p.values[0]);
					pMaterialPass->SetRenderPass(materialPassName);
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
					const auto slotIdx = std::stoi(std::move(p.values[0]));

				}
				else if (p.key == "Color")
				{
					colorProp = { std::stof(std::move(p.values[0])), std::stof(std::move(p.values[1])), std::stof(std::move(p.values[2])) };
					pmc.materialColor = colorProp;
				}
				else if (p.key == "Roughness")
				{
					roughnessProp = std::stof(std::move(p.values[0]));
					pmc.roughness = roughnessProp;
				}
				else if (p.key == "Texture")
				{
					// Format: Slot, TextureName
					const auto slotIdx = std::stoi(std::move(p.values[0]));
					const auto texPath = std::move(std::move(p.values[1]));

					pPropertySlot->AddBinding(Texture::Resolve(gfx, texPath))
						.SetupPSBinding(slotIdx);

					const auto pSampler = Sampler::Resolve(gfx);
					pPropertySlot->AddBinding(std::move(pSampler))
						.SetupPSBinding(slotIdx);
				}
			}
			else if (state == MaterialParseState::Pass)
			{
				if (p.key == "VS")
				{
					// Bind vertex shader and input layout
					const auto vertexShaderName = p.values[0];
					auto pVertexShader = VertexShader::Resolve(gfx, vertexShaderName.c_str());
					const auto pvsbc = pVertexShader->GetBytecode();
					auto pInputLayout = InputLayout::Resolve(gfx, std::move(m_vertexLayout), vertexShaderName, pvsbc);

					pMaterialPass->SetVertexShader(std::move(pVertexShader), std::move(pInputLayout));
				}
				else if (p.key == "PS")
				{
					pMaterialPass->SetPixelShader(PixelShader::Resolve(gfx, p.values[0].c_str()));
				}
				else if (p.key == "PropertySlot")
				{
					// Connects to previously defined properties
					const auto slotIdx = std::stoi(std::move(p.values[0]));
					pMaterialPass->SetPropertySlot(slotIdx);
				}
			}

		}
		parser.Dispose();

	}

	void Material::Bind(const GraphicsDevice& gfx, std::string_view passName)
	{
		// Is this needed?
	}

	void Material::SubmitDrawCommands(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		// Only submit draw calls for passes contained in draw context
		for (const auto& passName : drawContext.renderPasses)
		{
			if (m_pPasses.find(passName) != m_pPasses.end())
			{
				const auto& pMaterialPass = m_pPasses.at(passName);
				const auto propertySlotIdx = pMaterialPass->GetPropertySlot();
 				const BindingList* const pPropertySlot = (propertySlotIdx != -1) ? m_pPropertySlots[propertySlotIdx].get() : nullptr;
				pMaterialPass->SubmitDrawCommands(meshRenderer, drawContext, pPropertySlot);
			}
		}
	}

	const u64 Material::GetMaterialCode() const
	{
		return m_materialCode;
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