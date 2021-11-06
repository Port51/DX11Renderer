#pragma once
#include "Drawable.h"
#include "Material.h"
#include <string>

class MeshRenderer : public Drawable
{
public:
	MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::vector<std::shared_ptr<Bindable>> pBindables);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	mutable DirectX::XMFLOAT4X4 modelMatrix;
	std::string name;
	std::shared_ptr<Material> pMaterial; // keep separate from other bindables for now...
};