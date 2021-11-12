#include "Texture.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace wrl = Microsoft::WRL;

Texture::Texture(Graphics& gfx, const std::string& path, UINT slot)
	: path(path), slot(slot)
{
	SETUP_LOGGING(gfx);

	// load surface
	const auto s = Surface::FromFile(path);

	// todo: use staging texture for mips, and copy into immutable texture

	// create texture resource
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = s.GetWidth();
	textureDesc.Height = s.GetHeight();
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
		&textureDesc, nullptr, &pTexture
	));

	// write image data into top mip level
	GetContext(gfx)->UpdateSubresource(
		pTexture.Get(), 0u, nullptr, s.GetBufferPtrConst(), s.GetWidth() * sizeof(Surface::Color), 0u
	);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
		pTexture.Get(), &srvDesc, &pTextureView
	));

	GetContext(gfx)->GenerateMips(pTextureView.Get());
}

Texture::Texture(Graphics& gfx, const std::string& path, UINT slot, D3D11_TEXTURE2D_DESC textureDesc)
	: path(path), slot(slot)
{
	SETUP_LOGGING(gfx);

	// load surface
	const auto s = Surface::FromFile(path);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color); // distance in bytes between rows - keep in mind padding!
	wrl::ComPtr<ID3D11Texture2D> pTexture;

	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
		&textureDesc, &sd, &pTexture
	));

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
		pTexture.Get(), &srvDesc, &pTextureView
	));
}

Texture::Texture(Graphics& gfx, const std::string& path, UINT slot, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc)
	: path(path), slot(slot)
{
	SETUP_LOGGING(gfx);

	// load surface
	const auto s = Surface::FromFile(path);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color); // distance in bytes between rows - keep in mind padding!
	wrl::ComPtr<ID3D11Texture2D> pTexture;

	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
		&textureDesc, &sd, &pTexture
	));

	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
		pTexture.Get(), &srvDesc, &pTextureView
	));
}

void Texture::Bind(Graphics& gfx, UINT slot)
{
	GetContext(gfx)->PSSetShaderResources(slot, 1u, pTextureView.GetAddressOf());
}

std::string Texture::GetUID() const
{
	return GenerateUID(path, slot);
}

std::shared_ptr<Bindable> Texture::Resolve(Graphics& gfx, const std::string& path, UINT slot)
{
	return Bind::Codex::Resolve<Texture>(gfx, path, slot);
}

std::string Texture::GenerateUID(const std::string& path, UINT slot)
{
	using namespace std::string_literals;
	return typeid(Texture).name() + "#"s + path + "#" + std::to_string(slot);
}
