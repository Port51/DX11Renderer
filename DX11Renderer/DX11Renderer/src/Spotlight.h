#pragma once
#include "Light.h"
#include "LightShadowData.h"

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

	class Spotlight : public Light
	{
	public:
		Spotlight(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, const bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMFLOAT3 positionWS, const float pan, const float tilt, const dx::XMFLOAT3 color, const float intensity = 1.f, const float attenuationQ = 4.f, const float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		const LightData GetLightData(const dx::XMMATRIX viewMatrix) const override;
		const UINT GetLightType() const override;
		void RenderShadow(const ShadowPassContext& context, RenderState& renderState) override;
		void AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		const UINT GetShadowTileCount() const override;
	private:
		float m_attenuationQ;
		float m_range;
		float m_innerAngle = 80.f;
		float m_outerAngle = 31.f;
		LightShadowData m_lightShadowData;
	};
}