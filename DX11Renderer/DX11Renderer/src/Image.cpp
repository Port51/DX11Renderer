#include "pch.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./vendor/stb_image/stb_image.h"

Image::Image(const std::string& path)
{
	data = stbi_load(path.c_str(),
		&width,
		&height,
		&channels,
		desiredChannels);
	assert(data);

	pitch = width * 4u;




	// texture
	/*D3D11_TEXTURE2D_DESC image_texture_desc = {};

	image_texture_desc.Width = width;
	image_texture_desc.Height = height;
	image_texture_desc.MipLevels = 1;
	image_texture_desc.ArraySize = 1;
	image_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	image_texture_desc.SampleDesc.Count = 1;
	image_texture_desc.SampleDesc.Quality = 0;
	image_texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
	image_texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA image_subresource_data = {};

	image_subresource_data.pSysMem = data;
	image_subresource_data.SysMemPitch = pitch;

	ID3D11Texture2D *image_texture;

	result = device->CreateTexture2D(&image_texture_desc,
		&image_subresource_data,
		&image_texture
	);

	assert(SUCCEEDED(result));*/

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
