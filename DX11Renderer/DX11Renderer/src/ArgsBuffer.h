#pragma once
#include "Buffer.h"
#include "CommonHeader.h"
#include <wrl.h>

namespace gfx
{
	class GraphicsDevice;

	class ArgsBuffer : public Buffer
	{
	public:
		ArgsBuffer(const GraphicsDevice& gfx, const UINT setsOfArgs, const bool isIndexed);
	};
}