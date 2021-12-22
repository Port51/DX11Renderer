#pragma once
#include "Graphics.h"
#include "ModelInstance.h"
#include "Light.h"

class Renderer;
class Camera;
class RenderPass;
class DepthStencilTarget;
struct LightData;

class DirectionalLight : public Light
{
public:
	DirectionalLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
	void SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const override;
	UINT GetLightType() const override;
	void RenderShadow(Graphics& gfx, const Camera& cam, const std::unique_ptr<RenderPass>& pass, const std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB) override;
private:
	float pan;
	float tilt;
	float sphereRad;
	float range;
	std::unique_ptr<DepthStencilTarget> pShadowMap;
};