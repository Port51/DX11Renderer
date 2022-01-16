#include "pch.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./vendor/stb_image/stb_image.h"

namespace gfx
{
	Image::Image(const std::string& path)
	{
		data = stbi_load(path.c_str(),
			&width,
			&height,
			&channels,
			desiredChannels);
		assert(data);

		pitch = width * 4u;
	}

	Image::~Image()
	{
		free(data);
	}

	const unsigned char * Image::GetData() const
	{
		return data;
	}

	const UINT Image::GetPitch() const
	{
		return pitch;
	}

	const UINT Image::GetWidth() const
	{
		return width;
	}

	const UINT Image::GetHeight() const
	{
		return height;
	}
}