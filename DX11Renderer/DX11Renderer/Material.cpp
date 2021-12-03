#include "Material.h"
#include <typeinfo>
#include <string>
#include "Graphics.h"
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
#include "ConstantBuffer.h"
#include "NullPixelShader.h"

#include <fstream>
#include <sstream>

using namespace std::string_literals;

enum MaterialParseState { None, Properties, Pass };

// todo: pass actual VertexLayout class?
Material::Material(Graphics& gfx, const std::string_view _materialAssetPath)
	: materialAssetPath(std::string(_materialAssetPath))
{
	// NOTE: This is very WIP and should be replaced by something entirely different later
	// with most of the parsing logic moved to a different class

	// todo: make this not always the same
	vertexLayout
		.AppendVertexDesc<dx::XMFLOAT3>({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
		.AppendVertexDesc<dx::XMFLOAT3>({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
		.AppendVertexDesc<dx::XMFLOAT3>({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
		.AppendVertexDesc<dx::XMFLOAT2>({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })
		.AppendInstanceDesc<dx::XMFLOAT3>({ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }); // last = # instances to draw before moving onto next instance
	assert(vertexLayout.GetPerVertexStride() % 16 == 0);
	//assert(vertexLayout.GetPerInstanceStride() % 16 == 0);

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
	std::unique_ptr<Technique> pTechnique;
	std::unique_ptr<RenderStep> pPassStep;

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
				if (pPixelShader == nullptr)
				{
					pPassStep->AddBinding(Bind::NullPixelShader::Resolve(gfx))
						.SetupPSBinding(0u);
				}

				pTechnique->AddStep(std::move(pPassStep));
				pMaterialPass->AddTechnique(std::move(pTechnique));
				passes.emplace(materialPassName, std::move(pMaterialPass));

				// Error check
				if (pVertexShader == nullptr)
					throw std::runtime_error("Material at " + std::string(_materialAssetPath) + " is missing vertex shader!");
				
				pVertexShader = nullptr;
				pPixelShader = nullptr;
			}
		}
		else if (p.key == "Name")
		{
			if (state == MaterialParseState::Pass)
			{
				// Read pass name
				pPassStep = std::make_unique<RenderStep>(p.values[0]);
				materialPassName = std::move(p.values[0]);

				// Init cbuffer
				pPassStep->AddBinding(std::move(std::make_shared<ConstantBuffer<PSMaterialConstant>>(gfx, std::string(_materialAssetPath), pmc)))
					.SetupPSBinding(1u);
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
				pVertexShader = VertexShader::Resolve(gfx, vertexShaderName.c_str());
				const auto pvsbc = pVertexShader->GetBytecode();

				pPassStep->AddBinding(pVertexShader)
					.SetupVSBinding(0u);
				pPassStep->AddBinding(InputLayout::Resolve(gfx, std::move(vertexLayout), vertexShaderName, pvsbc))
					.SetupIABinding();
			}
			else if (p.key == "PS")
			{
				pPixelShader = PixelShader::Resolve(gfx, p.values[0].c_str());
				pPassStep->AddBinding(pPixelShader)
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

void Material::Bind(Graphics& gfx, std::string passName)
{
	// Is this needed?
}

void Material::SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, const MeshRenderer& renderer) const
{
	for (const auto& pass : passes)
	{
		pass.second->SubmitDrawCalls(frame, renderer);
	}
}

const VertexLayout& Material::GetVertexLayout() const
{
	return vertexLayout;
}

std::shared_ptr<Bindable> Material::Resolve(Graphics& gfx, const std::string_view assetPath)
{
	return Bind::Codex::Resolve<Material>(gfx, GenerateUID(assetPath), assetPath);
}

std::string Material::GenerateUID(const std::string_view assetPath)
{
	return typeid(Material).name() + "#"s + std::string(assetPath);
}

void Material::AddBindable(std::shared_ptr<Bindable> pBindable)
{
	pBindables.push_back(pBindable);
}
