#include "Material.h"
#include <typeinfo>
#include <string>
#include "Graphics.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Texture.h"
#include "Sampler.h"
#include "BindableCodex.h"
#include "MaterialPass.h"
#include "TransformCbuf.h"
#include "TextParser.h"
#include "RenderStep.h"
#include "Technique.h"
#include "Buffer.h"
#include "ConstantBuffer.h"

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
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Tangent)
		.Append(VertexLayout::Texcoord2D, 0u);

	DirectX::XMFLOAT3 colorProp = { 0.8f, 0.8f, 0.8f };
	float roughnessProp = 0.75f;

	MaterialParseState state = MaterialParseState::None;

	// For unpacking main properties
	struct PSMaterialConstant // must be multiple of 16 bytes
	{
		DirectX::XMFLOAT3 materialColor;
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
				pTechnique->AddStep(std::move(pPassStep));
				pMaterialPass->AddTechnique(std::move(pTechnique));
				passes.emplace(materialPassName, std::move(pMaterialPass));
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
				pVertexShader = VertexShader::Resolve(gfx, p.values[0].c_str());
				const auto pvsbc = pVertexShader->GetBytecode();

				pPassStep->AddBinding(pVertexShader)
					.SetupVSBinding(0u);
				pPassStep->AddBinding(InputLayout::Resolve(gfx, std::move(vertexLayout), pvsbc))
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

	if (pVertexShader == nullptr)
		throw std::runtime_error("Material at " + std::string(_materialAssetPath) + " is missing vertex shader!");
	if (pPixelShader == nullptr)
		throw std::runtime_error("Material at " + std::string(_materialAssetPath) + " is missing pixel shader!");

}

void Material::Bind(Graphics& gfx, std::string passName)
{
	// Execute pass if there is one
	/*const auto iter = passes.find(passName);
	if (iter != passes.end())
	{
		iter->second->Bind(gfx);
	}*/
}

std::string Material::GetUID() const
{
	return GenerateUID(materialAssetPath);
}

void Material::SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, const MeshRenderer& renderer) const
{
	for (const auto& pass : passes)
	{
		pass.second->SubmitDrawCalls(frame, renderer);
	}
}

const VertexLayout & Material::GetVertexLayout() const
{
	return vertexLayout;
}

std::shared_ptr<Bindable> Material::Resolve(Graphics & gfx, const std::string_view assetPath)
{
	return Bind::Codex::Resolve<Material>(gfx, assetPath);
}

std::string Material::GenerateUID(const std::string_view assetPath)
{
	return typeid(Material).name() + "#"s + std::string(assetPath);
}

void Material::AddBindable(std::shared_ptr<Bindable> pBindable)
{
	pBindables.push_back(pBindable);
}
