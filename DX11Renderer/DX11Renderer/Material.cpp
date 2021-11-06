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

#include <fstream>
#include <sstream>

using namespace std::string_literals;

// todo: pass actual VertexLayout class?
Material::Material(Graphics& gfx, const std::string_view assetPath)
	: assetPath(std::string(assetPath))
{
	DirectX::XMFLOAT3 colorProp = { 0.8f,0.8f,0.8f };
	float roughnessProp = 0.75f;

	// todo: move this to asset reader class
	std::ifstream file(std::string(assetPath).c_str());
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);

			// Read property key
			std::string key;
			getline(iss, key, ',');

			// Read settings
			std::vector<std::string> values;
			std::string token;
			while (getline(iss, token, ','))
			{
				// Remove spaces
				token.erase(std::remove_if(token.begin(), token.end(), isspace), token.end());
				values.push_back(token);
			}

			// Apply
			if (key == "VS")
			{
				// Bind vertex shader and input layout
				pVertexShader = std::make_shared<VertexShader>(gfx, values[0].c_str());
				auto pvsbc = pVertexShader->GetBytecode();
				AddBindable(pVertexShader);

				// moved:
				//pBindables.push_back(InputLayout::Resolve(gfx, _vertexLayout, pvsbc));
			}
			else if (key == "PS")
			{
				pPixelShader = PixelShader::Resolve(gfx, values[0].c_str());
				AddBindable(pPixelShader);
			}
			else if (key == "Color")
			{
				colorProp = { std::stof(values[0]), std::stof(values[1]), std::stof(values[2]) };
			}
			else if (key == "Roughness")
			{
				roughnessProp = std::stof(values[0]);
			}

		}
		file.close();
	}

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

	AddBindable(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, std::string(assetPath), pmc, 1u));

	AddBindable(Texture::Resolve(gfx, "Models\\HeadTextures\\face_albedo.png", 0u));
	AddBindable(Texture::Resolve(gfx, "Models\\HeadTextures\\face_normal.png", 1u));
	AddBindable(Sampler::Resolve(gfx));

	if (pVertexShader == nullptr)
		throw std::runtime_error("Material at " + std::string(assetPath) + " is missing vertex shader!");
	if (pPixelShader == nullptr)
		throw std::runtime_error("Material at " + std::string(assetPath) + " is missing pixel shader!");

}

void Material::Bind(Graphics& gfx)
{
	// Bind all resources
	for (auto& b : pBindables) // instance binds
	{
		b->Bind(gfx);
	}
}

std::string Material::GetUID() const
{
	return GenerateUID(assetPath);
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
