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
	class Graphics;
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
		LightManager(Graphics& gfx, std::shared_ptr<RendererList> pRendererList);
		virtual ~LightManager() = default;
	public:
		void AddLightModelsToList(RendererList& pRendererList);
		void CullLights(Graphics& gfx, const std::unique_ptr<Camera>& cam, bool enableShadows);
		std::unique_ptr<DepthStencilTarget>& GetShadowAtlas();
		std::unique_ptr<ConstantBuffer<LightInputCB>>& GetLightInputCB();
		void RenderShadows(ShadowPassContext context);
		ComPtr<ID3D11ShaderResourceView> GetLightDataSRV() const;
		ComPtr<ID3D11ShaderResourceView> GetShadowDataSRV() const;
		void DrawImguiControlWindows();
		UINT GetLightCount() const;
		std::shared_ptr<Light> GetLight(UINT index) const;
		UINT GetClusterCount() const;
		UINT GetClusterDimensionX() const;
		UINT GetClusterDimensionY() const;
		UINT GetClusterDimensionZ() const;
	private:
		bool FrustumSphereIntersection(dx::XMVECTOR lightSphere, dx::XMFLOAT4 frustumCorners, float farClipPlane);
		bool AABBSphereIntersection(const LightData& lightData, dx::XMVECTOR aabbCenter, dx::XMVECTOR aabbExtents);
	private:
		std::vector<std::shared_ptr<Light>> pLights;
		std::shared_ptr<DirectionalLight> pMainLight;
		std::vector<LightData> cachedLightData;
		UINT visibleLightCt;
	private:
		UINT clusterDimensionX;
		UINT clusterDimensionY;
		UINT clusterDimensionZ;
		std::unique_ptr<StructuredBuffer<LightData>> pLightData;
		std::unique_ptr<ConstantBuffer<LightInputCB>> pLightInputCB;
		std::unique_ptr<StructuredBuffer<LightShadowData>> pLightShadowSB;
		std::shared_ptr<RendererList> pShadowRendererList;
		std::vector<LightShadowData> cachedShadowData;
		std::unique_ptr<DepthStencilTarget> pShadowAtlas;
	};
}