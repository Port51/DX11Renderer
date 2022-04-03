#include "pch.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./vendor/stb_image/stb_image.h"

namespace gfx
{
	Image::Image(const std::string& path)
	{
		m_data = stbi_load(path.c_str(),
			&m_width,
			&m_height,
			&m_channels,
			m_desiredChannels);
		assert(m_data);

		m_pitch = m_width * 4u;
	}

	Image::~Image()
	{
		free(m_data);
	}

	const unsigned char * Image::GetData() const
	{
		return m_data;
	}

	const UINT Image::GetPitch() const
	{
		return m_pitch;
	}

	const UINT Image::GetWidth() const
	{
		return m_width;
	}

	const UINT Image::GetHeight() const
	{
		return m_height;
	}
}