#pragma once
#include "CommonHeader.h"
#include <vector>
#include "DXMathInclude.h"

struct ID3D11ShaderResourceView;

namespace gfx
{
	class Graphics;
	class Camera;
	class RenderPass;
	class ShadowPassContext;
	class ModelInstance;

	template<typename Type>
	class ConstantBuffer;

	struct LightData;
	struct LightShadowData;
	struct ShadowPassCB;

	class Light
	{
	public:
		struct ShadowSettings
		{
			bool hasShadow;
		};
		struct ShadowPassCB
		{

		};
	public:
		Light(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity);
	public:
		virtual void DrawImguiControlWindow() = 0;
		virtual LightData GetLightData(dx::XMMATRIX viewMatrix) const = 0;
		virtual UINT GetLightType() const = 0;
		virtual void RenderShadow(ShadowPassContext context) = 0;
		bool HasShadow() const;
		virtual void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData, std::vector<ComPtr<ID3D11ShaderResourceView>>& srvs) const = 0;
		virtual UINT GetShadowSRVCount() const = 0;
		ModelInstance& GetModelInstance() const;
	protected:
		UINT index;
		dx::XMFLOAT3 positionWS;
		dx::XMFLOAT3 color;
		float intensity;
		std::unique_ptr<ModelInstance> pModel;
		ShadowSettings shadowSettings;
		std::unique_ptr<ConstantBuffer<ShadowPassCB>> pShadowPassCB;
	};
}