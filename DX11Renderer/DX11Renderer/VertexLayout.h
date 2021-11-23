#pragma once
#include <DirectXMath.h>
#include <type_traits>
#include "Graphics.h"
#include "Colors.h"
#include "GraphicsThrowMacros.h"

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
		perVertexPadding = (16 - (perInstanceStride % 16)) % 16;
		code += desc.SemanticName + std::to_string(desc.SemanticIndex);
		return *this;
	}
	size_t GetPerVertexStride() const
	{
		return perVertexStride + perVertexPadding;
	}
	size_t GetPerInstanceStride() const
	{
		return perInstanceStride + perVertexPadding;
	}
	size_t GetPerVertexPadding() const
	{
		return perVertexPadding;
	}
	size_t GetPerInstancePadding() const
	{
		return perInstanceStride;
	}
	size_t GetElementCount() const
	{
		return elements.size();
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
	size_t perVertexStride = 0;
	size_t perInstanceStride = 0;
	size_t perVertexPadding = 0;
	std::string code = "";
};