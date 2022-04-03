#pragma once
#include "DXMathInclude.h"
#include <type_traits>
#include "GraphicsDevice.h"
#include "ColorStructs.h"

namespace gfx
{
	///
	/// Descriptor class for layout required by a vertex shader
	///
	class VertexLayout
	{
	public:
		VertexLayout() {}
		virtual ~VertexLayout() = default;
	public:
		template <class T> // T should be the CPU type
		VertexLayout& AppendVertexDesc(D3D11_INPUT_ELEMENT_DESC desc)
		{
			// Assign offset automatically
			desc.AlignedByteOffset = m_perVertexStride;

			m_elements.emplace_back(desc);
			m_perVertexStride += sizeof(T);
			m_perVertexPadding = (16 - (m_perVertexStride % 16)) % 16;
			m_code += desc.SemanticName + std::to_string(desc.SemanticIndex);
			return *this;
		}
		template <class T> // T should be the CPU type
		VertexLayout& AppendInstanceDesc(D3D11_INPUT_ELEMENT_DESC desc)
		{
			// Assign offset automatically
			desc.AlignedByteOffset = m_perInstanceStride;

			m_elements.emplace_back(desc);
			m_perInstanceStride += sizeof(T);
			m_code += desc.SemanticName + std::to_string(desc.SemanticIndex);
			return *this;
		}
		UINT GetPerVertexStride() const
		{
			return m_perVertexStride + m_perVertexPadding;
		}
		UINT GetPerInstanceStride() const
		{
			return m_perInstanceStride;
		}
		UINT GetPerVertexPadding() const
		{
			return m_perVertexPadding;
		}
		UINT GetElementCount() const
		{
			return (UINT)m_elements.size();
		}
		std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const
		{
			return m_elements;
		}
		std::string GetCode() const
		{
			return m_code;
		}

	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_elements;
		UINT m_perVertexStride = 0;
		UINT m_perInstanceStride = 0;
		UINT m_perVertexPadding = 0;
		std::string m_code = "";
	};
}