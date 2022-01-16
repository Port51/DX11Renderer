#pragma once

struct D3D11_SUBRESOURCE_DATA;

namespace gfx
{
	class BaseBufferData
	{
	public:
		virtual D3D11_SUBRESOURCE_DATA GetSubresourceData() const = 0;
		virtual UINT GetElementCount() const = 0;
		virtual UINT GetSizeInBytes() const = 0;
		virtual UINT GetStride() const = 0;
	};
}