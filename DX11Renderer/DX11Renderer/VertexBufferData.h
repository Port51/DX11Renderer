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
	VertexBufferData(const VertexLayout& layout) noexcept(!IS_DEBUG)
		: layout(layout)
	{}
	const char* GetData() const noexcept(!IS_DEBUG)
	{
		return buffer.data();
	}
	const VertexLayout& GetLayout() const noexcept
	{
		return layout;
	}
	// Vertex count
	size_t Size() const noexcept(!IS_DEBUG)
	{
		return buffer.size() / layout.Size();
	}
	size_t SizeBytes() const noexcept(!IS_DEBUG)
	{
		return buffer.size();
	}
	// Place at back (end) of buffer
	template<typename ...Params>
	void EmplaceBack(Params&&... params) noexcept(!IS_DEBUG)
	{
		assert(sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
		buffer.resize(buffer.size() + layout.Size());
		// Set all attributes by index
		// Will trigger param unwrapping, so index will be 0, 1, 2, 3, etc.
		Back().SetAttributeByIndex(0u, std::forward<Params>(params)...);
	}
	// Last
	Vertex Back() noexcept(!IS_DEBUG)
	{
		assert(buffer.size() != 0u);
		return Vertex{ buffer.data() + buffer.size() - layout.Size(),layout };
	}
	// First
	Vertex Front() noexcept(!IS_DEBUG)
	{
		assert(buffer.size() != 0u);
		return Vertex{ buffer.data(),layout };
	}
	Vertex operator[](size_t i) noexcept(!IS_DEBUG)
	{
		assert(i < Size());
		return Vertex{ buffer.data() + layout.Size() * i,layout };
	}
	ConstVertex Back() const noexcept(!IS_DEBUG)
	{
		return const_cast<VertexBufferData*>(this)->Back();
	}
	ConstVertex Front() const noexcept(!IS_DEBUG)
	{
		return const_cast<VertexBufferData*>(this)->Front();
	}
	ConstVertex operator[](size_t i) const noexcept(!IS_DEBUG)
	{
		return const_cast<VertexBufferData&>(*this)[i];
	}
private:
	std::vector<char> buffer; // vector of bytes
	const VertexLayout& layout; // structure of these bytes
};