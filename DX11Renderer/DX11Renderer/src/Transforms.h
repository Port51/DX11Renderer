#pragma once
#include "DXMathInclude.h"

namespace gfx
{
	struct Transforms
	{
		dx::XMMATRIX model;
		dx::XMMATRIX modelView;
		dx::XMMATRIX modelViewProj;
	};
}