#pragma once
#include "BaseBufferData.h"
#include <vector>

///
/// Contains actual instance data
///
template <class T>
class StructuredBufferData : public BaseBufferData
{
public:
	StructuredBufferData(const size_t elementCount)
		: elementCount(elementCount), stride(sizeof(T))
	{
		data.resize(elementCount);
		nextInputIdx = 0u;
	}
	D3D11_SUBRESOURCE_DATA GetSubresourceData() const override
	{
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = data.data();
		sd.SysMemPitch = 0;
		sd.SysMemSlicePitch = 0;
		return sd;
	}
	size_t GetElementCount() const override
	{
		return elementCount;
	}
	size_t GetSizeInBytes() const override
	{
		return elementCount * stride;
	}
	size_t GetStride() const override
	{
		return stride;
	}
	void EmplaceBack(T value)
	{
		data[nextInputIdx++] = std::move(value);
	}
private:
	std::vector<T> data; // vector of bytes
	const size_t stride; // structure of these bytes
	const size_t elementCount;
	size_t nextInputIdx;
};