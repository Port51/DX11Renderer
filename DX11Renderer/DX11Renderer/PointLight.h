#pragma once
#include "Light.h"
#include <vector>

class Renderer;
class Camera;
class RenderPass;
class DepthStencilTarget;
class ShadowPassContext;

struct LightData;

class PointLight : public Light
{
public:
	PointLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
	UINT GetLightType() const override;
	void RenderShadow(ShadowPassContext context) override;
private:
	float sphereRad;
	float range;
	std::vector<std::unique_ptr<DepthStencilTarget>> pShadowMaps;
};