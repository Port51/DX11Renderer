#pragma once
#include "BaseBufferData.h"
#include <vector>

namespace gfx
{
	class RawBufferData : public BaseBufferData
	{
	public:
		RawBufferData(const UINT elementCount, const UINT stride, const UINT padding)
			: m_elementCount(elementCount), m_stride(stride), m_padding(padding)
		{
			m_elements.resize(stride * elementCount);
			m_nextInput = m_elements.data();
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
			return (UINT)m_elements.size();
		}

		const UINT GetStride() const override
		{
			return m_stride;
		}

		template<class T>
		void EmplaceBack(const T value)
		{
			memcpy(m_nextInput, &value, sizeof(T));
			m_nextInput += sizeof(T);
		}
		void EmplacePadding()
		{
			m_nextInput += m_padding;
		}
	private:
		std::vector<char> m_elements;
		const UINT m_stride;
		const UINT m_padding;
		const UINT m_elementCount;
		char* m_nextInput;
	};
}