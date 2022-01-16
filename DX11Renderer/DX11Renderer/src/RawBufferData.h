#pragma once
#include "BaseBufferData.h"
#include <vector>

namespace gfx
{
	class RawBufferData : public BaseBufferData
	{
	public:
		RawBufferData(const UINT elementCount, const UINT stride, const UINT padding)
			: elementCount(elementCount), stride(stride), padding(padding)
		{
			elements.resize(stride * elementCount);
			nextInput = elements.data();
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
			return (UINT)elements.size();
		}
		UINT GetStride() const override
		{
			return stride;
		}
		template<class T>
		void EmplaceBack(const T value)
		{
			memcpy(nextInput, &value, sizeof(T));
			nextInput += sizeof(T);
		}
		void EmplacePadding()
		{
			nextInput += padding;
		}
	private:
		std::vector<char> elements;
		const UINT stride;
		const UINT padding;
		const UINT elementCount;
		char* nextInput;
	};
}