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
	VertexBufferData(const size_t vertCount, const size_t stride)
		: vertCount(vertCount), stride(stride)
	{
		buffer.resize(stride * vertCount);
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
	// Place at back (end) of buffer
	template<class T>
	void EmplaceBack(T value)
	{
		*reinterpret_cast<T*>(buffer.data() + nextInputOffset) = value;
		nextInputOffset += sizeof(T);
	}
private:
	std::vector<char> buffer; // vector of bytes
	const size_t stride; // structure of these bytes
	const size_t vertCount;
	size_t nextInputOffset = 0;
};