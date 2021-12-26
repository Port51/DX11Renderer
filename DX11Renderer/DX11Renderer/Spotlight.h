#pragma once
#include "Light.h"
#include "LightShadowData.h"

class Renderer;
class Camera;
class RenderPass;
class DepthStencilTarget;
class ShadowPassContext;

struct LightData;
struct LightShadowData;
struct ID3D11ShaderResourceView;

class Spotlight : public Light
{
public:
	Spotlight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow() override;
	LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
	UINT GetLightType() const override;
	void RenderShadow(ShadowPassContext context) override;
	void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData, std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& srvs) const override;
	UINT GetShadowSRVCount() const override;
private:
	dx::XMVECTOR GetDirectionWS() const;
private:
	float pan;
	float tilt;
	float sphereRad;
	float range;
	float innerCos = 0.9f;
	float outerCos = 0.8f;
	LightShadowData lightShadowData;
	std::unique_ptr<DepthStencilTarget> pShadowMap;
};