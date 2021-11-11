#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include "VertexLayout.h"
#include "Bindable.h"

class VertexShader;
class PixelShader;
class Material;

///
/// Collection of bindables needed to execute a material pass in a technique
///
class MaterialPass
{
public:
	MaterialPass();
public:
	void AddBindable(std::shared_ptr<Bindable> pBindable);
	void Bind(Graphics& gfx);
public:
	std::shared_ptr<VertexShader> pVertexShader;
	std::shared_ptr<PixelShader> pPixelShader;
private:
	std::vector<std::shared_ptr<Bindable>> pBindables; // non-shader bindables
	Material* pParent;
};