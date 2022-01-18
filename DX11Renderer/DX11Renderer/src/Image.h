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
		unsigned char* data;
		int width;
		int height;
		int channels;
		int desiredChannels = 4;
		UINT pitch;
	};
}