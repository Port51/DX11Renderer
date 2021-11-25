#pragma once
#include "BaseBufferData.h"
#include <vector>

class RawBufferData : public BaseBufferData
{
public:
	RawBufferData(const size_t elementCount, const size_t stride, const size_t padding)
		: elementCount(elementCount), stride(stride), padding(padding)
	{
		elements.resize(stride * elementCount);
		nextInput = elements.data();
	}
	D3D11_SUBRESOURCE_DATA GetSubresourceData() const override
	{
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = elements.data();
		sd.SysMemPitch = 0;			// only for 2D/3D textures
		sd.SysMemSlicePitch = 0;	// only for 3D textures
		return sd;
	}
	size_t GetElementCount() const override
	{
		return elementCount;
	}
	size_t GetSizeInBytes() const override
	{
		return elements.size();
	}
	size_t GetStride() const override
	{
		return stride;
	}
	template<class T>
	void EmplaceBack(const T value)
	{
		memcpy(nextInput, &value, sizeof(T));
		nextInput += sizeof(T);
	}
	void EmplacePadding()
	{
		nextInput += padding;
	}
private:
	std::vector<char> elements;
	const size_t stride;
	const size_t padding;
	const size_t elementCount;
	char* nextInput;
};