#pragma once
#include "CommonHeader.h"
#include <string>

namespace gfx
{
	class Image
	{
	public:
		Image(const std::string& path);
		virtual ~Image();
	public:
		const unsigned char* GetData() const;
		const UINT GetPitch() const;
		const UINT GetWidth() const;
		const UINT GetHeight() const;
	private:
		unsigned char* m_data;
		int m_width;
		int m_height;
		int m_channels;
		int m_desiredChannels = 4;
		UINT m_pitch;
	};
}