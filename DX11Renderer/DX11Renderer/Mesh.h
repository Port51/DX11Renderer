#pragma once
#include "TestObject.h"

class Mesh : public DrawableTemplated<Mesh>
{
public:
	Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> pBindables);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	mutable DirectX::XMFLOAT4X4 modelMatrix;
};