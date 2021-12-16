#pragma once
#include "Graphics.h"
#include "ModelInstance.h"

struct LightData;

class Light
{
public:
	virtual void DrawImguiControlWindow() = 0;
	virtual void SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const = 0;
	virtual LightData GetLightData(dx::FXMMATRIX viewMatrix) const = 0;
};