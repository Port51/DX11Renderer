#pragma once
#include "TestObject.h"

class Mesh : public DrawableTemplated<Mesh>
{
public:
	Mesh(Graphics& gfx, DirectX::XMFLOAT3 materialColor, DirectX::XMFLOAT3 scale);
	void Update(float dt) override;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	// model transform
	DirectX::XMFLOAT3X3 mt;
};