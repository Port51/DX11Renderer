#pragma once
#include <DirectXMath.h>

namespace gfx
{
	struct LightData
	{
		dx::XMVECTOR positionVS_range;
		dx::XMVECTOR color_intensity;
		dx::XMVECTOR directionVS;
		dx::XMVECTOR data0;
	};
}