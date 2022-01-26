#pragma once
#include "Light.h"

struct ID3D11ShaderResourceView;

namespace gfx
{
	class Renderer;
	class Camera;
	class RenderPass;
	class DepthStencilTarget;
	class ShadowPassContext;

	struct LightData;
	struct LightShadowData;
	struct ViewProjTransforms;

	class DirectionalLight : public Light
	{
	public:
		DirectionalLight(Graphics& gfx, UINT index, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
		UINT GetLightType() const override;
		void RenderShadow(ShadowPassContext context) override;
		void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		UINT GetShadowTileCount() const override;
		dx::XMVECTOR GetShadowCascadeSphere(UINT idx) const;
	private:
		dx::XMVECTOR GetDirectionWS() const;
		ViewProjTransforms GetShadowTransforms(dx::XMVECTOR cascadeSphereCenterWS, float cascadeDistance) const;
	private:
		float pan;
		float tilt;
		float sphereRad;
		float range;
		std::vector<LightShadowData> lightShadowData;
		std::vector<dx::XMVECTOR> shadowCascadeSpheres;
	};
}