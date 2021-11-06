#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include "Bindable.h"

class VertexLayout;

class Material : public Bindable
{
public:
	Material(Graphics& gfx, const std::string_view assetPath, const VertexLayout& vertexLayout);
	void Bind(Graphics& gfx);
	std::string GetUID() const override;
public:
	static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string_view assetPath, const VertexLayout& vertexLayout);
	static std::string GenerateUID(const std::string_view assetPath, const VertexLayout& vertexLayout);
private:
	std::vector<std::shared_ptr<Bindable>> pBindables; // shaders and such
	std::string assetPath;
	const VertexLayout& vertexLayout;
};