#pragma once
#include <DirectXMath.h>

struct LightData
{
	DirectX::XMVECTOR positionVS_range;
	DirectX::XMVECTOR color_intensity;
	DirectX::XMVECTOR direction;
	DirectX::XMVECTOR data0;
};