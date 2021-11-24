#pragma once
#include "BaseBufferData.h"
#include <vector>

///
/// Contains actual instance data
///
template <class T>
class StructuredBufferData : BaseBufferData
{
public:
	StructuredBufferData(const size_t elementCount, const size_t stride, const size_t padding)
		: elementCount(elementCount), stride(stride), padding(padding)
	{
		data.resize(elementCount);
		nextInput = data.data();
	}
	D3D11_SUBRESOURCE_DATA GetSubresourceData() const override
	{
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = buffer.data();
		return sd;
	}
	size_t GetInstanceCount() const override
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
	template<T>
	void EmplaceBack(T value)
	{
		data[nextInputIdx++] = std::move(value);
	}
private:
	std::vector<T> data; // vector of bytes
	const size_t stride; // structure of these bytes
	const size_t padding;
	const size_t elementCount;
	size_t nextInputIdx;
};