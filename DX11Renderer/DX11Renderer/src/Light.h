#pragma once
#include "GameObject.h"
#include "CommonHeader.h"
#include <vector>
#include "DXMathInclude.h"

struct ID3D11ShaderResourceView;

namespace gfx
{
	class GraphicsDevice;
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

	class Light : public GameObject
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
		Light(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMFLOAT3 positionWS, const dx::XMFLOAT3 color, const float intensity);
		virtual ~Light();
	public:
		virtual void DrawImguiControlWindow() = 0;
		virtual const LightData GetLightData(const dx::XMMATRIX& viewMatrix) const = 0;
		virtual const UINT GetLightType() const = 0;
		virtual void RenderShadow(const ShadowPassContext& context, RenderState& renderState) = 0;
		const int GetCurrentShadowIdx() const;
		void SetCurrentShadowIdx(const int shadowMapIdx);
		const bool HasShadow() const;
		virtual void AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const = 0;
		virtual const UINT GetShadowTileCount() const = 0;
		ModelInstance* GetModelInstance() const;
		void SetShadowMatrixTile(dx::XMMATRIX& shadowMatrix, const int tileX, const int tileY);
		const bool AllowUserControl() const;
	protected:
		UINT m_index;
		bool m_allowUserControl;
		dx::XMFLOAT3 m_color;
		float m_intensity;
		std::unique_ptr<ModelInstance> m_pModel;
		ShadowSettings m_shadowSettings;
		std::unique_ptr<ConstantBuffer<ShadowPassCB>> m_pShadowPassCB;
		int m_shadowAtlasTileIdx;
	};
}