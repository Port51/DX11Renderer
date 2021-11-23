#pragma once
#include "VertexLayout.h"
#include "Vertex.h"
#include "ConstVertex.h"

///
/// Contains actual data of a mesh
///
class VertexBufferData
{
public:
	VertexBufferData(const size_t vertCount, const size_t stride, const size_t padding)
		: vertCount(vertCount), stride(stride), padding(padding)
	{
		buffer.resize(stride * vertCount);
		nextInput = buffer.data();
	}
	const char* GetData() const
	{
		return buffer.data();
	}
	size_t GetVertexCount() const
	{
		return vertCount;
	}
	size_t GetSizeInBytes() const
	{
		return buffer.size();
	}
	size_t GetStride() const
	{
		return stride;
	}
	// Place at back (end) of buffer
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
	const size_t vertCount;
	char* nextInput;
};