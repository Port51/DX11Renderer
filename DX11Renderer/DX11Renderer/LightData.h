#pragma once
#include <DirectXMath.h>

struct LightData
{
	DirectX::XMVECTOR positionVS;
	float range;
	dx::XMFLOAT3 color;
	float intensity;
};