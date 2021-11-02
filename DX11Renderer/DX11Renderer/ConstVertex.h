#pragma once
#include "Vertex.h"

///
/// Read-only view of vertex data that exists in VertexBuffer
///
class ConstVertex
{
public:
	ConstVertex(const Vertex& v) noexcept(!IS_DEBUG)
		:
		vertex(v)
	{}
	template<VertexLayout::ElementType Type>
	const auto& Attr() const noexcept(!IS_DEBUG)
	{
		return const_cast<Vertex&>(vertex).Attr<Type>();
	}
private:
	Vertex vertex;
};