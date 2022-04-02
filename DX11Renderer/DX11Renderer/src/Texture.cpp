#include "pch.h"
#include "Texture.h"
#include "SharedCodex.h"
#include "RenderPass.h"
#include "Image.h"

namespace gfx
{
	// Don't initialize texture
	Texture::Texture(GraphicsDevice & gfx)
		: tag("?")
	{

	}

	// Create descriptors and pTexture
	Texture::Texture(GraphicsDevice & gfx, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc)
		: tag("?")
	{
		THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(
			&textureDesc, NULL, &pTexture
		));

		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pShaderResourceView
		));
	}

	// Load image from file and start off with default 2D texture settings
	// This should be used for 99% of model-related textures
	Texture::Texture(GraphicsDevice& gfx, const std::string& path)
		: tag(path)
	{
		const Image image(path);

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = image.GetWidth();
		textureDesc.Height = image.GetHeight();
		textureDesc.MipLevels = 0; // use all mips
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(
			&textureDesc, nullptr, &pTexture
		));

		// Write image data into top mip level
		gfx.GetContext()->UpdateSubresource(
			pTexture.Get(), 0u, nullptr, image.GetData(), image.GetPitch(), 0u
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1; // use all mips

		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pShaderResourceView
		));

		gfx.GetContext()->GenerateMips(pShaderResourceView.Get());
	}

	// Load image from file and use custom descriptors
	// Use this in special cases
	Texture::Texture(GraphicsDevice& gfx, const std::string& _path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc)
		: tag(_path)
	{
		// Read some info from image
		const Image image(_path);
		textureDesc.Width = image.GetWidth();
		textureDesc.Height = image.GetHeight();

		if (textureDesc.MipLevels > 1)
		{
			// Ensure mips can be generated
			textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

			THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(
				&textureDesc, nullptr, &pTexture
			));

			// Write image data into top mip level
			gfx.GetContext()->UpdateSubresource(
				pTexture.Get(), 0u, nullptr, image.GetData(), image.GetPitch(), 0u
			);

			gfx.GetContext()->GenerateMips(pShaderResourceView.Get());
		}
		else
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = image.GetData();
			sd.SysMemPitch = image.GetPitch(); // distance in bytes between rows - keep in mind padding!

			THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(
				&textureDesc, &sd, &pTexture
			));
		}

		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pShaderResourceView
		));
	}

	void Texture::BindCS(GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->CSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
	}

	void Texture::BindVS(GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->VSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
	}

	void Texture::BindPS(GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->PSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
	}

	ComPtr<ID3D11UnorderedAccessView> Texture::GetUAV(UINT mipSlice) const
	{
		return pUAV.size() > mipSlice ? pUAV[mipSlice] : nullptr;
	}

	Texture::TextureDimension Texture::GetDimension() const
	{
		return dimension;
	}

	ComPtr<ID3D11ShaderResourceView> Texture::GetSRV() const
	{
		return pShaderResourceView;
	}

	std::shared_ptr<Bindable> Texture::Resolve(GraphicsDevice& gfx, const std::string& path)
	{
		// Create 2D texture with default settings
		return std::move(Codex::Resolve<Texture>(gfx, GenerateUID(path), path));
	}

	std::string Texture::GenerateUID(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(Texture).name() + "#"s + tag;
	}
}