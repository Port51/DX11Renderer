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
		Spotlight(const GraphicsDevice& gfx, UINT index, bool allowUserControl, bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity = 1.f, float attenuationQ = 4.f, float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		const LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
		const UINT GetLightType() const override;
		void RenderShadow(ShadowPassContext context) override;
		void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		const UINT GetShadowTileCount() const override;
	private:
		const dx::XMVECTOR GetDirectionWS() const;
	private:
		float m_pan;
		float m_tilt;
		float m_attenuationQ;
		float m_range;
		float m_innerAngle = 80.f;
		float m_outerAngle = 31.f;
		LightShadowData m_lightShadowData;
	};
}