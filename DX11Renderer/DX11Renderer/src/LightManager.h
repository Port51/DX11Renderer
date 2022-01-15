#pragma once
#include "Common.h"
#include "DXMathInclude.h"
#include <vector>
#include <memory>
#include <wrl.h>
#include "ShadowPassContext.h"
#include "LightShadowData.h"

class Graphics;
class Camera;
class RendererList;
class Light;
class RenderPass;
class ShadowPassContext;

struct LightInputCB;
struct GlobalTransformCB;
struct LightData;
struct LightShadowData;
struct ID3D11ShaderResourceView;

template<typename Type>
class ConstantBuffer;
template<typename Type>
class StructuredBuffer;

class LightManager
{
private:
	const static int MaxLightCount = 256;
	const static int MaxShadowCount = 4;
public:
	LightManager(Graphics& gfx, std::shared_ptr<RendererList> pRendererList);
public:
	void AddLightModelsToList(RendererList& pRendererList);
	void CullLights(Graphics& gfx, const Camera& cam);
	void BindShadowMaps(Graphics& gfx, UINT startSlot) const;
	void RenderShadows(ShadowPassContext context);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetLightDataSRV() const;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowDataSRV() const;
	void DrawImguiControlWindows();
private:
	bool FrustumSphereIntersection(dx::XMVECTOR lightSphere, dx::XMFLOAT4 frustumCorners, float farClipPlane);
	bool AABBSphereIntersection(const LightData& lightData, dx::XMVECTOR aabbCenter, dx::XMVECTOR aabbExtents);
private:
	std::vector<std::shared_ptr<Light>> pLights;
	std::vector<LightData> cachedLightData;
	UINT visibleLightCt;
private:
	std::unique_ptr<StructuredBuffer<LightData>> pLightData;
	std::unique_ptr<ConstantBuffer<LightInputCB>> pLightInputCB;
	std::unique_ptr<StructuredBuffer<LightShadowData>> pLightShadowSB;
	std::shared_ptr<RendererList> pShadowRendererList;
	std::vector<LightShadowData> cachedShadowData;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> pShadowMapSRVs;
};