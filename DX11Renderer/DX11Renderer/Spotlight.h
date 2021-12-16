#pragma once
#include "Graphics.h"
#include "ModelInstance.h"
#include "Light.h"

class Renderer;
struct LightData;

class Spotlight : public Light
{
public:
	Spotlight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 rotation, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	void SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const override;
	LightData GetLightData(dx::FXMMATRIX viewMatrix) const override;
private:
	UINT index;
	dx::XMFLOAT3 positionWS;
	dx::XMFLOAT3 rotation;
	dx::XMFLOAT3 color;
	float intensity;
	float sphereRad;
	float range;

	std::unique_ptr<ModelInstance> pModel;
};