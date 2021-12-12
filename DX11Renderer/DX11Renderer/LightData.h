#pragma once
#include <DirectXMath.h>

struct LightData
{
	DirectX::XMVECTOR positionVS_Range; // range = w component
	dx::XMFLOAT3 color;
	float intensity;
};