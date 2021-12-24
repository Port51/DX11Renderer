#pragma once
#include "Light.h"

class Renderer;
class Camera;
class RenderPass;
class DepthStencilTarget;
class ShadowPassContext;

struct LightData;

class Spotlight : public Light
{
public:
	Spotlight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
	UINT GetLightType() const override;
	void RenderShadow(ShadowPassContext context) override;
private:
	dx::XMVECTOR GetDirectionWS() const;
private:
	float pan;
	float tilt;
	float sphereRad;
	float range;
	float innerCos = 0.9f;
	float outerCos = 0.8f;
	std::unique_ptr<DepthStencilTarget> pShadowMap;
};