#pragma once
#include "Graphics.h"
#include "Buffer.h"
#include "ModelInstance.h"

class FrameCommander;

class PointLight
{
public:
	PointLight(Graphics& gfx, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity = 1.f, float range = 2.5f);
	void DrawImguiControlWindow();
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame) const;
	void Bind(Graphics& gfx, dx::FXMMATRIX viewMatrix) const;
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
	mutable Buffer<PointLightCBuf> globalLightCbuf;
};