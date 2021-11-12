#include "Material.h"
#include <typeinfo>
#include <string>
#include "Graphics.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "PixelConstantBuffer.h"
#include "InputLayout.h"
#include "Texture.h"
#include "Sampler.h"
#include "BindableCodex.h"
#include "MaterialPass.h"
#include "TransformCbuf.h"

#include <fstream>
#include <sstream>

using namespace std::string_literals;

enum MaterialParseState { None, Properties, Pass };

bool isWhitespace(unsigned char c) {
	return (c == ' ' || c == '\n' || c == '\r' ||
		c == '\t' || c == '\v' || c == '\f');
}

// todo: pass actual VertexLayout class?
Material::Material(Graphics& gfx, const std::string_view assetPath)
	: assetPath(std::string(assetPath))
{
	// todo: make this not always the same
	vertexLayout
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Tangent)
		.Append(VertexLayout::Texture2D);
	
	DirectX::XMFLOAT3 colorProp = { 0.8f,0.8f,0.8f };
	float roughnessProp = 0.75f;

	// todo: move this to asset reader class and clean it up!
	std::ifstream file(std::string(assetPath).c_str());

	MaterialParseState state = MaterialParseState::None;

	// For unpacking material passes
	std::string materialPassName;
	std::unique_ptr<MaterialPass> pMaterialPass;
	std::unique_ptr<Technique> pTechnique;
	std::unique_ptr<Step> pPassStep;

	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			// Clean up line first
			line.erase(std::remove_if(line.begin(), line.end(), isWhitespace), line.end());

			std::istringstream iss(line);

			// Read property key
			std::string key;
			getline(iss, key, ',');

			// Read settings
			std::vector<std::string> values;
			std::string token;
			while (getline(iss, token, ','))
			{
				values.push_back(token);
			}

			// Apply
			if (key == "Properties")
			{
				state = MaterialParseState::Properties;
			}
			else if (key == "Pass")
			{
				state = MaterialParseState::Pass;
				pMaterialPass = std::make_unique<MaterialPass>();
				pTechnique = std::make_unique<Technique>();
			}
			else if (key == "}")
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
			else if (key == "Name")
			{
				if (state == MaterialParseState::Pass)
				{
					// Read pass name
					pPassStep = std::make_unique<Step>(values[0]);
					materialPassName = std::move(values[0]);
				}
			}
			else if (state == MaterialParseState::Properties)
			{
				if (key == "Color")
				{
					colorProp = { std::stof(values[0]), std::stof(values[1]), std::stof(values[2]) };
				}
				else if (key == "Roughness")
				{
					roughnessProp = std::stof(values[0]);
				}
				else if (key == "Texture")
				{
					// Format: Texture, PropName, Path
					auto texProp = std::move(values[0]);
				}
			}
			else if (state == MaterialParseState::Pass)
			{
				if (key == "VS")
				{
					// Bind vertex shader and input layout
					pVertexShader = std::make_shared<VertexShader>(gfx, values[0].c_str());
					auto pvsbc = pVertexShader->GetBytecode();

					// todo: choose one!?
					pMaterialPass->AddBinding(pVertexShader);
					pPassStep->AddBinding(pVertexShader);

					pPassStep->AddBinding(InputLayout::Resolve(gfx, vertexLayout, pvsbc));

					// moved:
					//pBindables.push_back(InputLayout::Resolve(gfx, _vertexLayout, pvsbc));
				}
				else if (key == "PS")
				{
					pPixelShader = PixelShader::Resolve(gfx, values[0].c_str());

					// todo: choose one!?
					pMaterialPass->AddBinding(pPixelShader);
					pPassStep->AddBinding(pPixelShader);
				}
				else if (key == "Texture")
				{
					// Format: Texture, PropName, Slot
					auto texProp = std::move(values[0]);
					auto slotIdx = std::stoi(std::move(values[1]));

					pPassStep->AddBinding(Texture::Resolve(gfx, "Models\\HeadTextures\\face_albedo.png", (UINT)slotIdx), (UINT)slotIdx);
					pPassStep->AddBinding(Sampler::Resolve(gfx));
				}
			}

		}
		file.close();
	}

	if (pVertexShader == nullptr)
		throw std::runtime_error("Material at " + std::string(assetPath) + " is missing vertex shader!");
	if (pPixelShader == nullptr)
		throw std::runtime_error("Material at " + std::string(assetPath) + " is missing pixel shader!");

	// todo: read from file
	struct PSMaterialConstant // must be multiple of 16 bytes
	{
		DirectX::XMFLOAT3 materialColor;
		float roughness;
		BOOL normalMappingEnabled = TRUE; // BOOL uses 4 bytes as it's an int, rather than bool
		float specularPower = 30.0f;
		float padding[2];
	} pmc;
	pmc.materialColor = colorProp;
	pmc.roughness = roughnessProp;

	std::shared_ptr<InputLayout> pInputLayout = InputLayout::Resolve(gfx, vertexLayout, pVertexShader->GetBytecode());

	AddBindable(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, std::string(assetPath), pmc, 1u));

	AddBindable(Texture::Resolve(gfx, "Models\\HeadTextures\\face_albedo.png", 0u));
	AddBindable(Texture::Resolve(gfx, "Models\\HeadTextures\\face_normal.png", 1u));
	AddBindable(Sampler::Resolve(gfx));

}

void Material::Bind(Graphics& gfx, std::string passName)
{
	// Execute pass if there is one
	const auto iter = passes.find(passName);
	if (iter != passes.end())
	{
		iter->second->Bind(gfx);
	}
}

std::string Material::GetUID() const
{
	return GenerateUID(assetPath);
}

void Material::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const
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
