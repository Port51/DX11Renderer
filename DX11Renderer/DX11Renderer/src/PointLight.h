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
		PointLight(const GraphicsDevice& gfx, UINT index, bool allowUserControl, bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity = 1.f, float attenuationQ = 4.f, float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		const LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
		const UINT GetLightType() const override;
		void RenderShadow(const ShadowPassContext&  context) override;
		void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		const UINT GetShadowTileCount() const override;
	private:
		float m_range;
		float m_attenuationQ;
		std::vector<LightShadowData> lightShadowData;
		static std::vector<dx::XMVECTOR> viewDirectionsWS;
	};
}