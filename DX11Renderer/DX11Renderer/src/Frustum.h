#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	struct Frustum
	{
	public:
		Frustum();
		Frustum(std::vector<dx::XMFLOAT4> planes);
	public:
		// XYZ = normal, W = distance
		std::vector<dx::XMFLOAT4> planes;
	};
}