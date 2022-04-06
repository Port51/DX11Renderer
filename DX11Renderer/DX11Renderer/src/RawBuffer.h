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
		RawBuffer(const GraphicsDevice& gfx, UINT bytes, D3D11_USAGE usage, UINT bindFlags);
	};
}