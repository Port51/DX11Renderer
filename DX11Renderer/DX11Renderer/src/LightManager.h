#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"
#include <vector>
#include "ShadowPassContext.h"

#include "StructuredBuffer.h"
#include "ConstantBuffer.h"
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

	struct LightInputCB;
	struct GlobalTransformCB;
	struct LightData;
	//struct LightShadowData;

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	class LightManager
	{
	public:
		const static UINT MaxLightCount = 256u;
		const static UINT ClusteredLightingClusterPixels = 16u;
		const static UINT ClusteredLightingZLevels = 16u;
		const static UINT MaxLightsPerCluster = 32u;
	public:
		LightManager(const GraphicsDevice& gfx, std::shared_ptr<RendererList> pRendererList);
		virtual ~LightManager() = default;
	public:
		void AddLightModelsToList(RendererList& pRendererList);
		void CullLightsAndShadows(const GraphicsDevice& gfx, const Camera& cam, bool enableShadows);
		std::unique_ptr<DepthStencilTarget>& GetShadowAtlas();
		std::unique_ptr<ConstantBuffer<LightInputCB>>& GetLightInputCB();
		void RenderShadows(ShadowPassContext context);
		const ComPtr<ID3D11ShaderResourceView> GetLightDataSRV() const;
		const ComPtr<ID3D11ShaderResourceView> GetShadowDataSRV() const;
		void DrawImguiControlWindows() const;
		const UINT GetLightCount() const;
		const std::shared_ptr<Light> GetLight(UINT index) const;
		const UINT GetClusterCount() const;
		const UINT GetClusterDimensionX() const;
		const UINT GetClusterDimensionY() const;
		const UINT GetClusterDimensionZ() const;
		const std::unique_ptr<StructuredBuffer<int>>& GetClusteredIndices() const;
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
		std::unique_ptr<StructuredBuffer<LightData>> m_pLightData;
		std::unique_ptr<ConstantBuffer<LightInputCB>> m_pLightInputCB;
		std::unique_ptr<StructuredBuffer<LightShadowData>> m_pLightShadowSB;
		std::shared_ptr<RendererList> m_pShadowRendererList;
		std::vector<LightShadowData> m_cachedShadowData;
		std::unique_ptr<DepthStencilTarget> m_pShadowAtlas;
		std::unique_ptr<StructuredBuffer<int>> m_pClusteredIndices;
	};
}