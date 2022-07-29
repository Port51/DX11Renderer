#include "pch.h"
#include "Material.h"
#include <typeinfo>
#include "GraphicsDevice.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "VertexAttributesLayout.h"
#include "Texture.h"
#include "Sampler.h"
#include "SharedCodex.h"
#include "MaterialPass.h"
#include "TransformCbuf.h"
#include "TextParser.h"
#include "VariableParser.h"
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

#include <fstream>
#include <sstream>
#include <assert.h>

namespace gfx
{
	using namespace std::string_literals;

	enum MaterialParseState { None, Properties, Attributes, Pass };

	// todo: move to GFXCore!
	const char* DeepCopyCStr(const std::string& string)
	{
		// https://stackoverflow.com/questions/18491883/deep-copy-stdstringc-str-to-char
		const std::string::size_type size = string.size();
		char* buffer = new char[size + 1]; // we need extra char for NUL
		memcpy(buffer, string.c_str(), size + 1);
		return buffer;
	}

	Material::Material(const GraphicsDevice& gfx, const std::string_view _materialAssetPath)
		: m_materialAssetPath(std::string(_materialAssetPath))
	{

		/*m_vertexLayout
			.AppendVertexDesc<dx::XMFLOAT3>({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT3>({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT4>({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendVertexDesc<dx::XMFLOAT2>({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
			.AppendInstanceDesc<dx::XMFLOAT3>({ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }); // last = # instances to draw before moving onto next instance
		assert(m_vertexLayout.GetPerVertexStride() % 16 == 0);
		*/

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
		ZERO_MEM(pmc);
		assert(sizeof(PSMaterialConstant) % 16 == 0);

		// For unpacking material passes
		RenderPassType materialPassRenderType = RenderPassType::Undefined;
		std::unique_ptr<MaterialPass> pMaterialPass;
		std::unique_ptr<BindingList> pPropertySlot;
		std::unique_ptr<VertexAttributesLayout> pAttributesSlot;
		size_t currentAttributeSlot = 0u;

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
			else if (p.key == "Attributes")
			{
				state = MaterialParseState::Attributes;
				pAttributesSlot = std::make_unique<VertexAttributesLayout>();
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
				else if (state == MaterialParseState::Attributes)
				{
					m_pAttributesSlots.emplace_back(std::move(pAttributesSlot));
				}
				else if (state == MaterialParseState::Pass)
				{
					// Init cbuffer
					pMaterialPass->AddBinding(std::move(std::make_shared<ConstantBuffer<PSMaterialConstant>>(gfx, D3D11_USAGE_IMMUTABLE, pmc)))
						.SetupPSBinding(RenderSlots::PS_FreeRendererCB + 0u);

					m_pMaterialPassesByType.emplace(materialPassRenderType, std::move(pMaterialPass));
				}
			}
			else if (p.key == "Name")
			{
				if (state == MaterialParseState::Pass)
				{
					// Read pass name and map to render pass
					pMaterialPass->SetName(p.values[0]);
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
					const auto slotIdx = p.MoveInt(0);
				}
				else if (p.key == "Color")
				{
					colorProp = { p.MoveFloat(0), p.MoveFloat(1), p.MoveFloat(2) };
					pmc.materialColor = colorProp;
				}
				else if (p.key == "Roughness")
				{
					roughnessProp = p.MoveFloat(0);
					pmc.roughness = roughnessProp;
				}
				else if (p.key == "Texture")
				{
					// Format: Slot, TextureName
					const auto slotIdx = p.MoveInt(0);
					const auto texPath = std::move(p.values[1]);

					pPropertySlot->AddBinding(Texture::Resolve(gfx, texPath))
						.SetupPSBinding(slotIdx);

					auto pSampler = Sampler::Resolve(gfx);
					pPropertySlot->AddBinding(std::move(pSampler))
						.SetupPSBinding(slotIdx);
				}
			}
			else if (state == MaterialParseState::Attributes)
			{
				if (p.key == "Slot")
				{
					const auto slotIdx = p.MoveInt(0);
				}
				else if (p.key == "I" || p.key == "V")
				{
					if (p.values.size() != 2u && p.values.size() != 3u)
					{
						THROW(std::string("Material '") + std::string(_materialAssetPath) + std::string("' has invalid attribute defined as '") + p.line + std::string("' which parsed into ") + std::to_string(p.values.size()) + std::string(" values!"));
					}

					// Example format: I, float3, POSITIONVS
					const D3D11_INPUT_CLASSIFICATION classification = (p.key == "I") ? D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
					const std::string& varType = p.ReadString(0);
					const std::string& varName = p.ReadString(1);
					const size_t bytes = VariableParser::GetByteSize(varType);

					// Offsets will be calculated later!
					D3D11_INPUT_ELEMENT_DESC desc;
					desc.SemanticName = DeepCopyCStr(varName);
					desc.SemanticIndex = (p.values.size() == 3u) ? std::stoi(p.ReadString(2)) : 0u;
					desc.Format = GetAttributeFormat(varType.c_str()); // DXGI_FORMAT
					desc.InputSlot = (classification == D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA) ? 1u : 0u;
					desc.AlignedByteOffset = 0u;
					desc.InputSlotClass = classification;
					desc.InstanceDataStepRate = (classification == D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA) ? 1u : 0u;

					if (classification == D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA)
					{
						pAttributesSlot->AppendInstanceDesc(desc, bytes);
					}
					else
					{
						pAttributesSlot->AppendVertexDesc(desc, bytes);
					}
				}
			}
			else if (state == MaterialParseState::Pass)
			{
				if (p.key == "VS")
				{
					if (pMaterialPass->GetAttributeSlot() == -1)
					{
						THROW(std::string("Material '") + std::string(_materialAssetPath) + std::string("' has pass ") + pMaterialPass->GetName() + std::string("' without an attribute slot index at the top! Please include something like: 'AttributeSlot, 0'"));
					}
					else if (m_pAttributesSlots.size() <= pMaterialPass->GetAttributeSlot())
					{
						THROW(std::string("Material '") + std::string(_materialAssetPath) + std::string("' does not have attribute slot #") + std::to_string((int)pMaterialPass->GetAttributeSlot()) + std::string("!"));
					}

					// Bind vertex shader and input layout
					const auto vertexShaderName = std::move(p.values[0]);
					auto pVertexShader = (p.values.size() == 1u) ?
						VertexShader::Resolve(gfx, vertexShaderName.c_str())
						: VertexShader::Resolve(gfx, vertexShaderName.c_str(), p.values[1].c_str());
					const auto pvsbc = pVertexShader->GetBytecode();
					auto pInputLayout = InputLayout::Resolve(gfx, *m_pAttributesSlots.at(pMaterialPass->GetAttributeSlot()).get(), vertexShaderName.c_str(), pvsbc);

					pMaterialPass->SetVertexShader(std::move(pVertexShader), std::move(pInputLayout));
				}
				else if (p.key == "PS")
				{
					const auto pixelShaderName = std::move(p.values[0]);
					auto pPixelShader = (p.values.size() == 1u) ?
						PixelShader::Resolve(gfx, pixelShaderName.c_str())
						: PixelShader::Resolve(gfx, pixelShaderName.c_str(), p.values[1].c_str());
					pMaterialPass->SetPixelShader(pPixelShader);
				}
				else if (p.key == "PropertySlot")
				{
					// Connects to previously defined properties
					const auto slotIdx = p.MoveInt(0);
					pMaterialPass->SetPropertySlot(slotIdx);
				}
				else if (p.key == "AttributeSlot")
				{
					const auto slotIdx = p.MoveInt(0);
					pMaterialPass->SetAttributeSlot(slotIdx);
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
		if (m_pMaterialPassesByType.find(drawContext.renderPass) != m_pMaterialPassesByType.end())
		{
			const auto& pMaterialPass = m_pMaterialPassesByType.at(drawContext.renderPass);
			const auto propertySlotIdx = pMaterialPass->GetPropertySlot();
 			const BindingList* const pPropertySlot = (propertySlotIdx != -1) ? m_pPropertySlots[propertySlotIdx].get() : nullptr;
			pMaterialPass->SubmitDrawCommands(drawable, drawContext, pPropertySlot);
		}
	}

	const size_t Material::GetAttributeSlotsCount() const
	{
		return m_pAttributesSlots.size();
	}

	const int Material::GetAttributesIndex(const RenderPassType renderPassType) const
	{
		return m_pMaterialPassesByType.at(renderPassType)->GetAttributeSlot();
	}

	const u64 Material::GetMaterialCode() const
	{
		return m_materialCode;
	}

	const VertexAttributesLayout& Material::GetVertexLayout(const size_t attributeSlotIdx) const
	{
		return *m_pAttributesSlots.at(attributeSlotIdx).get();
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

	const DXGI_FORMAT Material::GetAttributeFormat(const char* key) const
	{
		if (strcmp(key, "float4") == 0) return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (strcmp(key, "float3") == 0) return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		if (strcmp(key, "float2") == 0) return DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
		if (strcmp(key, "float") == 0)  return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
		THROW(std::string("Unrecognized attribute key '") + std::string(key) + std::string("'"));
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	}
}