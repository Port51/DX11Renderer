#pragma once

struct D3D11_SUBRESOURCE_DATA;

class BaseBufferData
{
public:
	virtual D3D11_SUBRESOURCE_DATA GetSubresourceData() const = 0;
	virtual size_t GetElementCount() const = 0;
	virtual size_t GetSizeInBytes() const = 0;
	virtual size_t GetStride() const = 0;
};