#include "pch.h"
#include "VertexAttributesLayout.h"

namespace gfx
{
	VertexAttributesLayout::VertexAttributesLayout() {}

	VertexAttributesLayout& VertexAttributesLayout::AppendVertexDesc(D3D11_INPUT_ELEMENT_DESC desc, const size_t sizeofElement)
	{
		// Assign offset automatically
		desc.AlignedByteOffset = m_perVertexStride;

		m_elements.emplace_back(desc);
		m_perVertexStride += sizeofElement;
		m_perVertexPadding = (16u - (m_perVertexStride % 16u)) % 16u;
		m_code += desc.SemanticName + std::to_string(desc.SemanticIndex);
		return *this;
	}

	VertexAttributesLayout& VertexAttributesLayout::AppendInstanceDesc(D3D11_INPUT_ELEMENT_DESC desc, const size_t sizeofElement)
	{
		// Assign offset automatically
		desc.AlignedByteOffset = m_perInstanceStride;

		m_elements.emplace_back(desc);
		m_perInstanceStride += sizeofElement;
		m_code += desc.SemanticName + std::to_string(desc.SemanticIndex);
		return *this;
	}

	const UINT VertexAttributesLayout::GetPerVertexStride() const
	{
		return m_perVertexStride + m_perVertexPadding;
	}

	const UINT VertexAttributesLayout::GetPerInstanceStride() const
	{
		return m_perInstanceStride;
	}

	const UINT VertexAttributesLayout::GetPerVertexPadding() const
	{
		return m_perVertexPadding;
	}

	const UINT VertexAttributesLayout::GetElementCount() const
	{
		return (UINT)m_elements.size();
	}

	const bool VertexAttributesLayout::HasPosition() const
	{
		return m_hasPosition;
	}

	const bool VertexAttributesLayout::HasNormal() const
	{
		return m_hasNormal;
	}

	const bool VertexAttributesLayout::HasTangent() const
	{
		return m_hasTangent;
	}

	const bool VertexAttributesLayout::HasTexcoord(const size_t idx) const
	{
		return idx < m_texcoordCount;
	}

	const u32 VertexAttributesLayout::GetTexcoordCount() const
	{
		return m_texcoordCount;
	}

	const std::vector<D3D11_INPUT_ELEMENT_DESC> VertexAttributesLayout::GetD3DLayout() const
	{
		return m_elements;
	}

	const std::string VertexAttributesLayout::GetCode() const
	{
		return m_code;
	}
}