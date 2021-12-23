#pragma once
#include "Graphics.h"
#include "ModelInstance.h"
#include "ConstantBuffer.h"

class Camera;
class RenderPass;
class ShadowPassContext;

struct LightData;

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
	virtual void RenderShadow(ShadowPassContext context, Graphics& gfx, const Camera& cam, const std::unique_ptr<RenderPass>& pass, const std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB) = 0;
	bool HasShadow() const;
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