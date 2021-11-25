#pragma once
#include "BaseBufferData.h"
#include <vector>

///
/// Contains actual data of a mesh
///
class VertexBufferData : public BaseBufferData
{
public:
	VertexBufferData(const size_t elementCount, const size_t stride, const size_t padding)
		: elementCount(elementCount), stride(stride), padding(padding)
	{
		buffer.resize(stride * elementCount);
		nextInput = buffer.data();
	}
	D3D11_SUBRESOURCE_DATA GetSubresourceData() const override
	{
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = buffer.data();
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
		return buffer.size();
	}
	size_t GetStride() const override
	{
		return stride;
	}
	template<class T>
	void EmplaceBack(T value)
	{
		memcpy(nextInput, &value, sizeof(T));
		nextInput += sizeof(T);
	}
	void EmplacePadding()
	{
		nextInput += padding;
	}
private:
	std::vector<char> buffer; // vector of bytes
	const size_t stride; // structure of these bytes
	const size_t padding;
	const size_t elementCount;
	char* nextInput;
};