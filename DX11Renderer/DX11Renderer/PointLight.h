#pragma once
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "ModelInstance.h"

class FrameCommander;
struct LightData;

class PointLight
{
public:
	PointLight(Graphics& gfx, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity = 1.f, float range = 2.5f);
public:
	void DrawImguiControlWindow();
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame) const;
	//void Bind(Graphics& gfx, dx::FXMMATRIX viewMatrix) const;
	LightData GetLightData(dx::FXMMATRIX viewMatrix) const;
private:
	struct PointLightCBuf
	{
		//alignas(16) 
		dx::XMFLOAT3 positionVS;
		float invRangeSqr;
		dx::XMFLOAT3 color;
		float intensity;
	};
private:
	PointLightCBuf cbData;
	dx::XMFLOAT3 positionWS;
	dx::XMFLOAT3 color;
	float intensity;
	float range;

	std::unique_ptr<ModelInstance> pModel;
	mutable ConstantBuffer<PointLightCBuf> globalLightCbuf;
};