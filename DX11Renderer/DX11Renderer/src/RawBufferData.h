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
			m_nextInputByteIdx = 0u;
		}

		~RawBufferData()
		{
			
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
#if defined(_DEBUG)
			if ((m_nextInputByteIdx + sizeof(T) - 1) >= m_elements.size())
			{
				THROW("RawBufferData: Element exceeded byte limit! Requested bytes " + std::to_string(m_nextInputByteIdx) + std::string(" to ") + std::to_string(m_nextInputByteIdx + sizeof(T) - 1) + std::string(" and there are only ") + std::to_string(m_elements.size()) + std::string(" bytes available!"));
			}
#endif
			memcpy(&m_elements.data()[m_nextInputByteIdx], &value, sizeof(T));
			m_nextInputByteIdx += sizeof(T);
		}

		void EmplacePadding()
		{
			m_nextInputByteIdx += m_padding;
		}

	private:
		std::vector<char> m_elements;
		const UINT m_stride;
		const UINT m_padding;
		const UINT m_elementCount;
		size_t m_nextInputByteIdx;
	};
}