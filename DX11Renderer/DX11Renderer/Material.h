#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include <unordered_map>
#include "VertexLayout.h"
#include "Bindable.h"

class VertexShader;
class PixelShader;
class MaterialPass;

class Material : public Bindable
{
public:
	Material(Graphics& gfx, const std::string_view assetPath);
	void Bind(Graphics& gfx, std::string passName);
	std::string GetUID() const override;
public:
	const VertexLayout& GetVertexLayout() const;
	static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string_view assetPath);
	static std::string GenerateUID(const std::string_view assetPath);
private:
	void AddBindable(std::shared_ptr<Bindable> pBindable);
public:
	std::shared_ptr<VertexShader> pVertexShader;
	std::shared_ptr<PixelShader> pPixelShader;
private:
	std::unordered_map<std::string, std::unique_ptr<MaterialPass>> passes;
	std::vector<std::shared_ptr<Bindable>> pBindables; // shaders and such
	std::string assetPath;
	VertexLayout vertexLayout;
};