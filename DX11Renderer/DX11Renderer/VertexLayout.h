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
	VertexLayout& Append(D3D11_INPUT_ELEMENT_DESC desc, size_t bytes)
	{
		elements.emplace_back(desc);
		size += bytes;
		code += desc.SemanticName + std::to_string(desc.SemanticIndex);
		return *this;
	}
	// Size in bytes
	size_t SizeInBytes() const
	{
		return size;
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
	size_t size = 0;
	std::string code = "";
};