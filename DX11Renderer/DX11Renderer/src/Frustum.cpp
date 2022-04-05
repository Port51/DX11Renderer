#include "pch.h"
#include "Frustum.h"

namespace gfx
{

	Frustum::Frustum()
	{
		planes.resize(6);
	}

	Frustum::Frustum(std::vector<dx::XMFLOAT4> planes)
		: planes(planes)
	{
		assert(planes.size() == 6 && "Frustum must have 6 planes");
	}

}