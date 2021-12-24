#pragma once
#include "Light.h"

class Renderer;
class Camera;
class RenderPass;
class DepthStencilTarget;
class ShadowPassContext;

struct LightData;

class DirectionalLight : public Light
{
public:
	DirectionalLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
	UINT GetLightType() const override;
	void RenderShadow(ShadowPassContext context) override;
private:
	float pan;
	float tilt;
	float sphereRad;
	float range;
	std::unique_ptr<DepthStencilTarget> pShadowMap;
};