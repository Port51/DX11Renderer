#pragma once
#include "Buffer.h"
#include "CommonHeader.h"
#include <wrl.h>

namespace gfx
{
	class GraphicsDevice;

	class RawBuffer : public Buffer
	{
	public:
		RawBuffer(const GraphicsDevice& gfx, const UINT bytes, const D3D11_USAGE usage, const UINT bindFlags);
	};
}