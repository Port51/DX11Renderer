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
			: elementCount(elementCount), stride(sizeof(T))
		{
			elements.resize(elementCount);
			nextInputIdx = 0u;
		}
		StructuredBufferData(std::vector<T> _elements)
			: elementCount(_elements.size()), stride(sizeof(T)), elements(std::move(_elements))
		{
			nextInputIdx = _elements.size();
		}
		D3D11_SUBRESOURCE_DATA GetSubresourceData() const override
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = elements.data();
			sd.SysMemPitch = 0;			// only for 2D/3D textures
			sd.SysMemSlicePitch = 0;	// only for 3D textures
			return sd;
		}
		UINT GetElementCount() const override
		{
			return elementCount;
		}
		UINT GetSizeInBytes() const override
		{
			return elementCount * stride;
		}
		UINT GetStride() const override
		{
			return stride;
		}
		void EmplaceBack(const T value)
		{
			assert(nextInputIdx < elements.size() && "StructuredBufferData out of range!");
			elements[nextInputIdx++] = std::move(value);
		}
	private:
		std::vector<T> elements;
		const UINT stride;
		const UINT elementCount;
		UINT nextInputIdx;
	};
}