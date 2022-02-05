#pragma once
#include "Buffer.h"
#include "CommonHeader.h"
#include <wrl.h>

namespace gfx
{
	class Graphics;

	class RawBuffer : public Buffer
	{
	public:
		RawBuffer(Graphics& gfx, UINT bytes, D3D11_USAGE usage, UINT bindFlags);
	};
}