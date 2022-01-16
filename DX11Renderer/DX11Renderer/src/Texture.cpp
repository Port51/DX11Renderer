#include "pch.h"
#include "Texture.h"
#include "SharedCodex.h"
#include "RenderPass.h"
#include "Image.h"

namespace gfx
{
	Texture::Texture(Graphics & gfx)
	{

	}

	Texture::Texture(Graphics& gfx, const std::string& path)
		: path(path)
	{
		// Load image from file
		const Image image(path);

		// todo: use staging texture for mips, and copy into immutable texture

		// create texture resource
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = image.GetWidth();
		textureDesc.Height = image.GetHeight();
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		THROW_IF_FAILED(gfx.GetDevice()->CreateTexture2D(
			&textureDesc, nullptr, &pTexture
		));

		// write image data into top mip level
		gfx.GetContext()->UpdateSubresource(
			pTexture.Get(), 0u, nullptr, image.GetData(), image.GetPitch(), 0u
		);

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		THROW_IF_FAILED(gfx.GetDevice()->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pShaderResourceView
		));

		gfx.GetContext()->GenerateMips(pShaderResourceView.Get());
	}

	Texture::Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc)
		: path(path)
	{
		// Load image from file
		const Image image(path);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = image.GetData();
		sd.SysMemPitch = image.GetPitch(); // distance in bytes between rows - keep in mind padding!
		ComPtr<ID3D11Texture2D> pTexture;

		THROW_IF_FAILED(gfx.GetDevice()->CreateTexture2D(
			&textureDesc, &sd, &pTexture
		));

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		THROW_IF_FAILED(gfx.GetDevice()->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pShaderResourceView
		));
	}

	Texture::Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc)
		: path(path)
	{
		// Load image from file
		const Image image(path);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = image.GetData();
		sd.SysMemPitch = image.GetPitch(); // distance in bytes between rows - keep in mind padding!
		ComPtr<ID3D11Texture2D> pTexture;

		THROW_IF_FAILED(gfx.GetDevice()->CreateTexture2D(
			&textureDesc, &sd, &pTexture
		));

		THROW_IF_FAILED(gfx.GetDevice()->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pShaderResourceView
		));
	}

	void Texture::BindCS(Graphics& gfx, UINT slot)
	{
		gfx.GetContext()->CSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
	}

	void Texture::BindVS(Graphics& gfx, UINT slot)
	{
		gfx.GetContext()->VSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
	}

	void Texture::BindPS(Graphics& gfx, UINT slot)
	{
		gfx.GetContext()->PSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
	}

	ComPtr<ID3D11UnorderedAccessView> Texture::GetUAV() const
	{
		return pUAV;
	}

	ComPtr<ID3D11ShaderResourceView> Texture::GetSRV() const
	{
		return pShaderResourceView;
	}

	std::shared_ptr<Bindable> Texture::Resolve(Graphics& gfx, const std::string& path)
	{
		return Codex::Resolve<Texture>(gfx, GenerateUID(path), path);
	}

	std::string Texture::GenerateUID(const std::string& path)
	{
		using namespace std::string_literals;
		return typeid(Texture).name() + "#"s + path;
	}
}