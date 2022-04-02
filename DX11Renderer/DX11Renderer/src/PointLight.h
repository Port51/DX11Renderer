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
		PointLight(GraphicsDevice& gfx, UINT index, bool allowUserControl, bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity = 1.f, float attenuationQ = 4.f, float range = 2.5f);
	public:
		void DrawImguiControlWindow() override;
		LightData GetLightData(dx::XMMATRIX viewMatrix) const override;
		UINT GetLightType() const override;
		void RenderShadow(ShadowPassContext context) override;
		void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const override;
		UINT GetShadowTileCount() const override;
	private:
		float range;
		float attenuationQ;
		std::vector<LightShadowData> lightShadowData;
		static std::vector<dx::XMVECTOR> viewDirectionsWS;
	};
}