#pragma once
#include "Light.h"
#include <vector>

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

	class PointLight : public Light
	{
	public:
		PointLight(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, const bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMFLOAT3 positionWS, const dx::XMFLOAT3 color, const float intensity = 1.f, const float attenuationQ = 4.f, const float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		const LightData GetLightData(const dx::XMMATRIX viewMatrix) const override;
		const UINT GetLightType() const override;
		void RenderShadow(const ShadowPassContext& context, RenderState& renderState) override;
		void AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		const UINT GetShadowTileCount() const override;
	private:
		float m_range;
		float m_attenuationQ;
		std::vector<LightShadowData> lightShadowData;
		static std::vector<dx::XMVECTOR> viewDirectionsWS;
	};
}