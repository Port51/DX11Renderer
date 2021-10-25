#pragma once
#include "DrawableTemplated.h"

class SolidSphere : public DrawableTemplated<SolidSphere>
{
public:
	SolidSphere(Graphics& gfx, float radius);
	void Update(float dt) override;
	void SetPos(DirectX::XMFLOAT3 pos);
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	DirectX::XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
};