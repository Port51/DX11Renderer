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
	VertexBufferData(const VertexLayout& layout, const size_t vertCount)
		: layout(layout), vertCount(vertCount)
	{
		buffer.resize(layout.Size() * vertCount);
	}
	const char* GetData() const
	{
		return buffer.data();
	}
	const VertexLayout& GetLayout() const
	{
		return layout;
	}
	// Vertex count
	size_t Size() const
	{
		return buffer.size() / layout.Size();
	}
	size_t SizeBytes() const
	{
		return buffer.size();
	}
	// Place at back (end) of buffer
	template<typename ...Params>
	void EmplaceBack(Params&&... params)
	{
		assert(sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
		// Set all attributes by index
		// Will trigger param unwrapping, so index will be 0, 1, 2, 3, etc.

		Vertex(buffer.data() + layout.Size() * nextInputVertex, layout).SetAttributeByIndex(0u, std::forward<Params>(params)...);
		nextInputVertex++;
	}
	// Last
	Vertex Back()
	{
		assert(buffer.size() != 0u);
		return Vertex(buffer.data() + buffer.size() - layout.Size(), layout);
	}
	// First
	Vertex Front()
	{
		assert(buffer.size() != 0u);
		return Vertex(buffer.data(), layout);
	}
	Vertex operator[](size_t i)
	{
		assert(i < Size());
		return Vertex(buffer.data() + layout.Size() * i, layout);
	}
	/*ConstVertex Back() const
	{
		return const_cast<VertexBufferData*>(this)->Back();
	}
	ConstVertex Front() const
	{
		return const_cast<VertexBufferData*>(this)->Front();
	}
	ConstVertex operator[](size_t i) const
	{
		return const_cast<VertexBufferData&>(*this)[i];
	}*/
private:
	std::vector<char> buffer; // vector of bytes
	const VertexLayout& layout; // structure of these bytes
	const size_t vertCount;
	size_t nextInputVertex = 0;
};