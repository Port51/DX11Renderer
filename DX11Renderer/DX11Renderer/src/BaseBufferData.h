#pragma once

struct D3D11_SUBRESOURCE_DATA;

namespace gfx
{
	class BaseBufferData
	{
	public:
		virtual const D3D11_SUBRESOURCE_DATA GetSubresourceData() const = 0;
		virtual const UINT GetElementCount() const = 0;
		virtual const UINT GetSizeInBytes() const = 0;
		virtual const UINT GetStride() const = 0;
	};
}