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
	class ModelAsset;

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
		Light(Graphics& gfx, UINT index, bool allowUserControl, std::shared_ptr<ModelAsset> const& pModelAsset, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity);
		virtual ~Light();
	public:
		virtual void DrawImguiControlWindow() = 0;
		virtual LightData GetLightData(dx::XMMATRIX viewMatrix) const = 0;
		virtual UINT GetLightType() const = 0;
		virtual void RenderShadow(ShadowPassContext context) = 0;
		int GetCurrentShadowIdx() const;
		void SetCurrentShadowIdx(int shadowMapIdx);
		dx::XMFLOAT3 GetPositionWS() const;
		void SetPositionWS(dx::XMVECTOR positionWS);
		bool HasShadow() const;
		virtual void AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const = 0;
		virtual UINT GetShadowTileCount() const = 0;
		ModelInstance& GetModelInstance() const;
		void SetShadowMatrixTile(dx::XMMATRIX& shadowMatrix, int tileX, int tileY);
		bool AllowUserControl() const;
	protected:
		UINT index;
		bool allowUserControl;
		dx::XMFLOAT3 positionWS;
		dx::XMFLOAT3 color;
		float intensity;
		std::unique_ptr<ModelInstance> pModel;
		ShadowSettings shadowSettings;
		std::unique_ptr<ConstantBuffer<ShadowPassCB>> pShadowPassCB;
		int shadowAtlasTileIdx;
	};
}