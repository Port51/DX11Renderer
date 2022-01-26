#pragma once
#include "DXMathInclude.h"

namespace gfx
{
	struct LightShadowData
	{
		dx::XMMATRIX shadowMatrix;
		dx::XMUINT2 tile;
		dx::XMFLOAT2 padding;
		//float shadowMapSize;
	};
}