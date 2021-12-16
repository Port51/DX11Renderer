#pragma once
#include "Graphics.h"
#include "ModelInstance.h"

struct LightData;

class Light
{
public:
	Light(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity);
public:
	virtual void DrawImguiControlWindow() = 0;
	virtual LightData GetLightData(dx::FXMMATRIX viewMatrix) const = 0;
	virtual void SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const;
protected:
	UINT index;
	dx::XMFLOAT3 positionWS;
	dx::XMFLOAT3 color;
	float intensity;
	std::unique_ptr<ModelInstance> pModel;
};