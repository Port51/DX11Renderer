#pragma once
#include "DXMathInclude.h"
#include <type_traits>
#include "Graphics.h"
#include "Colors.h"

///
/// Descriptor class for layout required by a vertex shader
///
class VertexLayout
{
public:
	VertexLayout() {}
public:
	template <class T> // T should be the CPU type
	VertexLayout& AppendVertexDesc(D3D11_INPUT_ELEMENT_DESC desc)
	{
		// Assign offset automatically
		desc.AlignedByteOffset = perVertexStride;

		elements.emplace_back(desc);
		perVertexStride += sizeof(T);
		perVertexPadding = (16 - (perVertexStride % 16)) % 16;
		code += desc.SemanticName + std::to_string(desc.SemanticIndex);
		return *this;
	}
	template <class T> // T should be the CPU type
	VertexLayout& AppendInstanceDesc(D3D11_INPUT_ELEMENT_DESC desc)
	{
		// Assign offset automatically
		desc.AlignedByteOffset = perInstanceStride;

		elements.emplace_back(desc);
		perInstanceStride += sizeof(T);
		code += desc.SemanticName + std::to_string(desc.SemanticIndex);
		return *this;
	}
	UINT GetPerVertexStride() const
	{
		return perVertexStride + perVertexPadding;
	}
	UINT GetPerInstanceStride() const
	{
		return perInstanceStride;
	}
	UINT GetPerVertexPadding() const
	{
		return perVertexPadding;
	}
	UINT GetElementCount() const
	{
		return (UINT)elements.size();
	}
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const
	{
		return elements;
	}
	std::string GetCode() const
	{
		return code;
	}

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
	UINT perVertexStride = 0;
	UINT perInstanceStride = 0;
	UINT perVertexPadding = 0;
	std::string code = "";
};