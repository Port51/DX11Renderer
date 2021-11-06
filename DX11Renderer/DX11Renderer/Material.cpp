#include "Material.h"
#include <typeinfo>
#include <string>
#include "Graphics.h"
#include "VertexShader.h"
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
			}

		}
		file.close();
	}
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
