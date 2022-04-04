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
	class ModelAsset;

	struct LightData;
	struct LightShadowData;
	struct ViewProjTransforms;

	class DirectionalLight : public Light
	{
	public:
		DirectionalLight(GraphicsDevice& gfx, UINT index, bool allowUserControl, bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float sphereRad = 1.f, float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		const LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
		const UINT GetLightType() const override;
		void RenderShadow(ShadowPassContext context) override;
		void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		const UINT GetShadowTileCount() const override;
		const dx::XMVECTOR GetShadowCascadeSphereVS(UINT idx) const;
	private:
		const dx::XMVECTOR GetDirectionWS() const;
		const ViewProjTransforms GetShadowTransforms(dx::XMVECTOR cascadeSphereCenterWS, float cascadeDistance) const;
	private:
		float m_pan;
		float m_tilt;
		float m_sphereRad;
		float m_range;
		std::vector<LightShadowData> m_lightShadowData;
		std::vector<dx::XMVECTOR> m_shadowCascadeSpheresVS;
	};
}