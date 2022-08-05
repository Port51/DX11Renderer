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
		DirectionalLight(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, const bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, const float pan, const float tilt, const dx::XMFLOAT3 color, const float intensity = 1.f, const float sphereRad = 1.f, const float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		const LightData GetLightData(const dx::XMMATRIX viewMatrix) const override;
		const UINT GetLightType() const override;
		void RenderShadow(const ShadowPassContext& context, RenderState& renderState) override;
		void AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		const UINT GetShadowTileCount() const override;
		const dx::XMVECTOR GetShadowCascadeSphereVS(const UINT idx) const;
	private:
		const ViewProjTransforms GetShadowTransforms(const dx::XMVECTOR cascadeSphereCenterWS, const float cascadeDistance) const;
	private:
		float m_sphereRad;
		float m_range;
		std::vector<LightShadowData> m_lightShadowData;
		std::vector<dx::XMVECTOR> m_shadowCascadeSpheresVS;
	};
}