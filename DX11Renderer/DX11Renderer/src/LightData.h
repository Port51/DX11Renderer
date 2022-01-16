#pragma once
#include <DirectXMath.h>

namespace gfx
{
	struct LightData
	{
		DirectX::XMVECTOR positionVS_range;
		DirectX::XMVECTOR color_intensity;
		DirectX::XMVECTOR directionVS;
		DirectX::XMVECTOR data0;
	};
}