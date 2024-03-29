#pragma once
#include "BaseBufferData.h"
#include <vector>
#include <assert.h>

namespace gfx
{
	template <class T>
	class StructuredBufferData : public BaseBufferData
	{
	public:
		StructuredBufferData(const UINT elementCount)
			: m_elementCount(elementCount), m_stride(sizeof(T))
		{
			m_elements.resize(elementCount);
			m_nextInputIdx = 0u;
		}

		StructuredBufferData(std::vector<T> _elements)
			: m_elementCount(_elements.size()), m_stride(sizeof(T)), m_elements(std::move(_elements))
		{
			m_nextInputIdx = _elements.size();
		}

		const D3D11_SUBRESOURCE_DATA GetSubresourceData() const override
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = m_elements.data();
			sd.SysMemPitch = 0;			// only for 2D/3D textures
			sd.SysMemSlicePitch = 0;	// only for 3D textures
			return sd;
		}

		const UINT GetElementCount() const override
		{
			return m_elementCount;
		}

		const UINT GetSizeInBytes() const override
		{
			return m_elementCount * m_stride;
		}

		const UINT GetStride() const override
		{
			return m_stride;
		}

		std::vector<T>& GetElements() const
		{
			return m_elements;
		}

		T GetElement(const size_t i) const
		{
			return m_elements.at(i);
		}

		void SetElement(const size_t i, const T data)
		{
			m_elements[i] = data;
		}

		void EmplaceBack(const T value)
		{
			assert(m_nextInputIdx < m_elements.size() && "StructuredBufferData out of range!");
			m_elements[m_nextInputIdx++] = std::move(value);
		}

	private:
		std::vector<T> m_elements;
		const UINT m_stride;
		const UINT m_elementCount;
		UINT m_nextInputIdx;
	};
}