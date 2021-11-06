#include "Material.h"
#include <typeinfo>
#include <string>
#include "Graphics.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "PixelConstantBuffer.h"
#include "InputLayout.h"

#include <fstream>
#include <sstream>

using namespace std::string_literals;

// todo: pass actual VertexLayout class?
Material::Material(Graphics& gfx, const std::string_view assetPath, const VertexLayout& _vertexLayout)
	: assetPath(std::string(assetPath)),
	vertexLayout(_vertexLayout)
	//: assetPath({ assetPath.begin(), assetPath.end() })
{
	bool hasVertexShader = false;
	bool hasPixelShader = false;
	DirectX::XMFLOAT3 colorProp = { 0.8f,0.8f,0.8f };

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
				auto pvs = std::make_shared<VertexShader>(gfx, values[0].c_str());
				auto pvsbc = pvs->GetBytecode();
				pBindables.push_back(std::move(pvs));

				pBindables.push_back(InputLayout::Resolve(gfx, _vertexLayout, pvsbc));
				hasVertexShader = true;
			}
			else if (key == "PS")
			{
				pBindables.push_back(PixelShader::Resolve(gfx, values[0].c_str()));
				hasPixelShader = true;
			}
			else if (key == "Color")
			{
				colorProp = { std::stof(values[0]), std::stof(values[1]), std::stof(values[2]) };
			}

		}
		file.close();
	}

	// todo: read from file
	struct PSMaterialConstant
	{
		DirectX::XMFLOAT3 color;
		float specularIntensity = 0.6f;
		float specularPower = 30.0f;
		float padding[3];
	} pmc;
	pmc.color = colorProp;

	pBindables.push_back(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, pmc, 1u));

	if (!hasVertexShader)
		throw std::runtime_error("Material at " + std::string(assetPath) + " is missing vertex shader!");
	if (!hasPixelShader)
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
	return GenerateUID(assetPath, vertexLayout);
}

std::shared_ptr<Bindable> Material::Resolve(Graphics & gfx, const std::string_view assetPath, const VertexLayout& vertexLayout)
{
	return Bind::Codex::Resolve<Material>(gfx, assetPath, vertexLayout);
}

std::string Material::GenerateUID(const std::string_view assetPath, const VertexLayout& vertexLayout)
{
	return typeid(Material).name() + "#"s + std::string(assetPath);
}
