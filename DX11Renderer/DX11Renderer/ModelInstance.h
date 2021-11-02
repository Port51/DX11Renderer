#pragma once
#include "TestObject.h"

class ModelInstance : public DrawableTemplated<ModelInstance>
{
public:
	ModelInstance(Graphics& gfx, DirectX::XMFLOAT3 materialColor, DirectX::XMFLOAT3 scale);
	void Update(float dt) override;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	// model transform
	DirectX::XMFLOAT3X3 mt;
};