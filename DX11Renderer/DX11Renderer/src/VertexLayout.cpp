#include "pch.h"
#include "VertexLayout.h"

namespace gfx
{
	VertexLayout::VertexLayout()
	{}

	VertexLayout::~VertexLayout()
	{}
	
	const UINT VertexLayout::GetPerVertexStride() const
	{
		return m_perVertexStride + m_perVertexPadding;
	}
	
	const UINT VertexLayout::GetPerInstanceStride() const
	{
		return m_perInstanceStride;
	}
	
	const UINT VertexLayout::GetPerVertexPadding() const
	{
		return m_perVertexPadding;
	}
	
	const UINT VertexLayout::GetElementCount() const
	{
		return (UINT)m_elements.size();
	}
	
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& VertexLayout::GetD3DLayout() const
	{
		return m_elements;
	}
	
	const std::string& VertexLayout::GetCode() const
	{
		return m_code;
	}
}

