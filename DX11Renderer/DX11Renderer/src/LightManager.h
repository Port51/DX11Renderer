#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"
#include <vector>

#include "LightShadowData.h"
#include "CommonCbuffers.h"
#include "LightData.h"

struct ID3D11ShaderResourceView;

namespace gfx
{
	class GraphicsDevice;
	class Camera;
	class RendererList;
	class Light;
	class RenderPass;
	class ShadowPassContext;
	class DepthStencilTarget;
	class DirectionalLight;
	class ConstantBuffer;
	class StructuredBuffer;

	struct LightInputCB;
	struct GlobalTransformCB;
	struct LightData;
	//struct LightShadowData;

	class LightManager
	{
	public:
		const static UINT MaxLightCount = 256u;
		const static UINT ClusteredLightingClusterPixels = 16u;
		const static UINT ClusteredLightingZLevels = 16u;
		const static UINT MaxLightsPerCluster = 32u;

	public:
		LightManager(const GraphicsDevice& gfx, const std::shared_ptr<RendererList>& pRendererList);
		virtual ~LightManager();
		void Release();

	public:
		void AddLight(std::shared_ptr<Light> pLight);
		void AddPointLight(const GraphicsDevice& gfx, const dx::XMVECTOR positionWS, const dx::XMFLOAT3 color, const float intensity = 1.f, const float attenuationQ = 4.f, const float range = 2.5f);
		void AddDirectionalLight(const GraphicsDevice& gfx, const float pan, const float tilt, const dx::XMFLOAT3 color, const float intensity = 1.f);
		void AddLightModelsToList(RendererList& pRendererList);
		void CullLightsAndShadows(const GraphicsDevice& gfx, const Camera& cam, const bool enableShadows);
		DepthStencilTarget& GetShadowAtlas();
		ConstantBuffer& GetLightInputCB();
		void RenderShadows(ShadowPassContext& context, RenderState& renderState);
		const ComPtr<ID3D11ShaderResourceView>& GetLightDataSRV() const;
		const ComPtr<ID3D11ShaderResourceView>& GetShadowDataSRV() const;
		void DrawImguiControlWindows() const;
		const UINT GetLightCount() const;
		Light& GetLight(const UINT index) const;
		const UINT GetClusterCount() const;
		const UINT GetClusterDimensionX() const;
		const UINT GetClusterDimensionY() const;
		const UINT GetClusterDimensionZ() const;
		const StructuredBuffer& GetClusteredIndices() const;

	private:
		std::vector<bool> m_lightVisibility;
		std::vector<std::shared_ptr<Light>> m_pLights;
		std::shared_ptr<DirectionalLight> m_pMainLight;
		std::vector<LightData> m_cachedLightData;
		UINT m_visibleLightCt;

	private:
		UINT m_clusterDimensionX;
		UINT m_clusterDimensionY;
		UINT m_clusterDimensionZ;
		std::unique_ptr<StructuredBuffer> m_pLightData;
		std::unique_ptr<ConstantBuffer> m_pLightInputCB;
		std::unique_ptr<StructuredBuffer> m_pLightShadowSB;
		std::shared_ptr<RendererList> m_pShadowRendererList;
		std::vector<LightShadowData> m_cachedShadowData;
		std::unique_ptr<DepthStencilTarget> m_pShadowAtlas;
		std::unique_ptr<StructuredBuffer> m_pClusteredIndices;

	};
}