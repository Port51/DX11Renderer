#pragma once
#include "Graphics.h"
#include "ModelInstance.h"
#include "Light.h"

class Renderer;
struct LightData;

class PointLight : public Light
{
public:
	PointLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::FXMMATRIX viewMatrix) const override;
	UINT GetLightType() const override;
private:
	float sphereRad;
	float range;
};