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
#include "RenderStep.h"
#include "Technique.h"
#include "Buffer.h"
#include "Binding.h"
#include "ConstantBuffer.h"
#include "NullPixelShader.h"
#include "RenderConstants.h"
#include "DrawContext.h"
#include "RasterizerState.h"

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
		struct PSMaterialConstant // must be multiple of 16 bytes
		{
			dx::XMFLOAT3 materialColor;
			float roughness;
			BOOL normalMappingEnabled = TRUE; // BOOL uses 4 bytes as it's an int, rather than bool
			float specularPower = 30.0f;
			float padding[2];
		} pmc;
		std::unordered_map<std::string, std::shared_ptr<Bindable>> pTexturesByPropName;

		//AddBindable(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, std::string(assetPath), pmc, 1u));

		// For unpacking material passes
		std::string materialPassName;
		std::unique_ptr<MaterialPass> pMaterialPass;

		TextParser parser(_materialAssetPath);
		TextParser::ParsedKeyValues p;
		while (parser.ReadParsedLine(p))
		{
			// Apply
			if (p.key == "Properties")
			{
				state = MaterialParseState::Properties;
			}
			else if (p.key == "Pass")
			{
				state = MaterialParseState::Pass;
				pMaterialPass = std::make_unique<MaterialPass>();
				pTechnique = std::make_unique<Technique>();
			}
			else if (p.key == "}")
			{
				// End scope
				if (state == MaterialParseState::Pass)
				{
					// End pass

					// If no pixel shader, bind null
					if (m_pPixelShader == nullptr)
					{
						pMaterialPass->SetPixelShader(nullptr);
					}

					m_pPasses.emplace(materialPassName, std::move(pMaterialPass));

					// Error check
					if (m_pVertexShader == nullptr)
						throw std::runtime_error("Material at " + std::string(_materialAssetPath) + " is missing vertex shader!");

					m_pVertexShader = nullptr;
					m_pPixelShader = nullptr;
				}
			}
			else if (p.key == "Name")
			{
				if (state == MaterialParseState::Pass)
				{
					// Read pass name
					materialPassName = std::move(p.values[0]);
					pMaterialPass->SetRenderPass(materialPassName);

					// Init cbuffer
					// todo: add to codex using std::string(_materialAssetPath)?
					pPassStep->AddBinding(std::move(std::make_shared<ConstantBuffer<PSMaterialConstant>>(gfx, D3D11_USAGE_IMMUTABLE, pmc)))
						.SetupPSBinding(RenderSlots::PS_FreeRendererCB + 0u);
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
				if (p.key == "Color")
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
					// Format: Texture, PropName, Path
					const auto texProp = std::move(p.values[0]);
					pTexturesByPropName.emplace(texProp, Texture::Resolve(gfx, p.values[1].c_str()));
				}
			}
			else if (state == MaterialParseState::Pass)
			{
				if (p.key == "VS")
				{
					// Bind vertex shader and input layout
					const auto vertexShaderName = p.values[0];
					m_pVertexShader = VertexShader::Resolve(gfx, vertexShaderName.c_str());
					const auto pvsbc = m_pVertexShader->GetBytecode();

					pMaterialPass->SetVertexShader(m_pVertexShader);
					pPassStep->AddBinding(m_pVertexShader)
						.SetupVSBinding(0u);
					pPassStep->AddBinding(InputLayout::Resolve(gfx, std::move(m_vertexLayout), vertexShaderName, pvsbc))
						.SetupIABinding();
				}
				else if (p.key == "PS")
				{
					m_pPixelShader = PixelShader::Resolve(gfx, p.values[0].c_str());
					pPassStep->AddBinding(m_pPixelShader)
						.SetupPSBinding(0u);
				}
				else if (p.key == "Texture")
				{
					// Format: Texture, PropName, Slot
					const auto texProp = std::move(std::move(p.values[0]));
					const auto slotIdx = std::stoi(std::move(p.values[1]));

					const auto iter = pTexturesByPropName.find(texProp);
					if (iter != pTexturesByPropName.end())
					{
						pPassStep->AddBinding(iter->second)
							.SetupPSBinding(slotIdx);
					}
					else
					{
						throw std::runtime_error("Material at " + std::string(_materialAssetPath) + " does not have texture property " + texProp + "!");
					}

					const auto pSampler = Sampler::Resolve(gfx);
					pPassStep->AddBinding(std::move(pSampler))
						.SetupPSBinding(slotIdx);
				}
			}

		}
		parser.Dispose();

	}

	void Material::Bind(const GraphicsDevice& gfx, std::string_view passName)
	{
		// Is this needed?
	}

	void Material::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		// Only submit draw calls for passes contained in draw context
		for (const auto& passName : drawContext.renderPasses)
		{
			if (m_pPasses.find(passName) != m_pPasses.end())
			{
				m_pPasses.at(passName)->SubmitDrawCall(meshRenderer, drawContext);
			}
		}
	}

	const u64 Material::GetMaterialCode() const
	{
		// Create sorting code that minimizes state changes
		// Cost determines the order:
		// Shader program > ROP > texture bindings > vertex format > UBO bindings > vert bindings > uniform updates

		// Actually use 48 bits for this, as 16 will be used for depth
		// 10 bits - pixel shader (1024 possible)
		// 10 bits - vertex shader (1024 possible)
		// 2 bits - rasterizer state (4 possible)
		// 12 bits - texture bindings
		// 8 bits - vertex layout (256 possible)
		// 6 bits - UBO bindings (64 possible)
		auto ps = (m_pPixelShader != nullptr) ? m_pPixelShader->GetInstanceIdx() : 0u;
		auto vs = (m_pVertexShader != nullptr) ? m_pVertexShader->GetInstanceIdx() : 0u;
		return ps << 38u
			+ vs << 28u;
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