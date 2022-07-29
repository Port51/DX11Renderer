#pragma once
#include "CommonHeader.h"
#include "DX11Include.h"
#include "DXMathInclude.h"
#include <type_traits>
#include "GraphicsDevice.h"
#include "ColorStructs.h"

namespace gfx
{
	///
	/// Descriptor class for layout required by a vertex shader
	///
	class VertexAttributesLayout
	{
	public:
		VertexAttributesLayout();
		virtual ~VertexAttributesLayout() = default;

	public:
		VertexAttributesLayout& AppendVertexDesc(D3D11_INPUT_ELEMENT_DESC desc, const size_t sizeofElement);
		VertexAttributesLayout& AppendInstanceDesc(D3D11_INPUT_ELEMENT_DESC desc, const size_t sizeofElement);

	public:
		const UINT GetPerVertexStride() const;
		const UINT GetPerInstanceStride() const;
		const UINT GetPerVertexPadding() const;
		const UINT GetElementCount() const;
		const bool HasPosition() const;
		const bool HasNormal() const;
		const bool HasTangent() const;
		const bool HasTexcoord(const size_t idx) const;
		const u32 GetTexcoordCount() const;
		const std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const;
		const std::string GetCode() const;

	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_elements;
		UINT m_perVertexStride = 0;
		UINT m_perInstanceStride = 0;
		UINT m_perVertexPadding = 0;
		bool m_hasPosition;
		bool m_hasNormal;
		bool m_hasTangent;
		u32 m_texcoordCount;
		std::string m_code = "";
	};
}