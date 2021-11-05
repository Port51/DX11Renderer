#pragma once
#include "Drawable.h"
#include "Material.h"
#include <string>

class Mesh : public Drawable
{
public:
	Mesh(Graphics& gfx, std::string name, std::vector<std::shared_ptr<Bindable>> pBindables);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	mutable DirectX::XMFLOAT4X4 modelMatrix;
	std::string name;
	// todo: replace with codex
	std::unique_ptr<Material> pMaterial;
};