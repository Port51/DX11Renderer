#pragma once
#include <vector>
#include <type_traits>
#include "Graphics.h"
#include "VertexLayout.h"

///
/// Read-only view of vertex that exists in VertexBuffer
///
class Vertex
{
	friend class VertexBufferData;
public:
	// View into attribute of vertex
	// Usage: vBuffer[vertIdx].Attr<Pos3D>() = { 0, 0, 0 }
	template<VertexLayout::ElementType Type>
	auto& Attr()(!IS_DEBUG)
	{
		// pData = ptr to start of data
		auto pAttribute = pData + layout.Resolve<Type>().GetOffset();
		return *reinterpret_cast<typename VertexLayout::Map<Type>::SysType*>(pAttribute);
	}
	// T&& = universal reference, used in perfect forwarding
	// Allows forwarding param without losing any info or modifiers on it
	// std::forward() forwards as-is
	template<typename T>
	void SetAttributeByIndex(size_t i, T&& val)(!IS_DEBUG)
	{
		const auto& element = layout.ResolveByIndex(i);
		auto pAttribute = pData + element.GetOffset();
		switch (element.GetType())
		{
		case VertexLayout::Position2D:
			SetAttribute<VertexLayout::Position2D>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::Position3D:
			SetAttribute<VertexLayout::Position3D>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::Texture2D:
			SetAttribute<VertexLayout::Texture2D>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::Normal:
			SetAttribute<VertexLayout::Normal>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::Tangent:
			SetAttribute<VertexLayout::Tangent>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::Float3Color:
			SetAttribute<VertexLayout::Float3Color>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::Float4Color:
			SetAttribute<VertexLayout::Float4Color>(pAttribute, std::forward<T>(val));
			break;
		case VertexLayout::BGRAColor:
			SetAttribute<VertexLayout::BGRAColor>(pAttribute, std::forward<T>(val));
			break;
		default:
			THROW_GFX_EXCEPT("Unrecognized vertex element type");
		}
	}
public:
	Vertex(char* pData, const VertexLayout& layout)(!IS_DEBUG)
		:
		pData(pData),
		layout(layout)
	{
		assert(pData != nullptr);
	}
private:
	template<typename First, typename ...Rest>
	// enables parameter pack setting of multiple parameters by element index
	// make sure to only call this if 2+ params!
	void SetAttributeByIndex(size_t i, First&& first, Rest&&... rest)(!IS_DEBUG)
	{
		// keep peeling one off and calling again for others, until all done
		// last call will be other overload of SetAttributeByIndex()
		SetAttributeByIndex(i, std::forward<First>(first));
		SetAttributeByIndex(i + 1, std::forward<Rest>(rest)...); // if Rest has 1 param, will resolve to other SetAttributeByIndex()
	}
	// helper to reduce code duplication in SetAttributeByIndex
	template<VertexLayout::ElementType DestLayoutType, typename SrcType>
	void SetAttribute(char* pAttribute, SrcType&& val)(!IS_DEBUG)
	{
		using Dest = typename VertexLayout::Map<DestLayoutType>::SysType;
		// SrcType = from T&&
		// Dest = something like Position2D
		if constexpr (std::is_assignable<Dest, SrcType>::value)
		{
			*reinterpret_cast<Dest*>(pAttribute) = val;
		}
		else
		{
			THROW_GFX_EXCEPT("Parameter attribute type mismatch");
		}
	}
private:
	char* pData = nullptr;
	const VertexLayout& layout;
};
