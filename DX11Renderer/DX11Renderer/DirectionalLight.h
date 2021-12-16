#pragma once
#include "Graphics.h"
#include "ModelInstance.h"
#include "Light.h"

class Renderer;
struct LightData;

class DirectionalLight : public Light
{
public:
	DirectionalLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::FXMMATRIX viewMatrix) const override;
	void SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const override;
	UINT GetLightType() const override;
private:
	float pan;
	float tilt;
	float sphereRad;
	float range;
};