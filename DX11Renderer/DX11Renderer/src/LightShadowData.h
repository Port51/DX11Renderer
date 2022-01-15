#pragma once
#include "DXMathInclude.h"

struct LightShadowData
{
	dx::XMMATRIX shadowMatrix;
	dx::XMMATRIX lightViewMatrix;
	dx::XMMATRIX lightViewProjMatrix;
	//float shadowMapSize;
	//dx::XMFLOAT3 padding;
};