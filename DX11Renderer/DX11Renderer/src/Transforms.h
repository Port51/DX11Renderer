#pragma once
#include "DXMathInclude.h"

namespace gfx
{

	struct ViewProjTransforms
	{
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projMatrix;
	};
}