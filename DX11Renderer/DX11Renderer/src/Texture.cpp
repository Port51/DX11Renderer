#include "pch.h"
#include "Texture.h"
#include "SharedCodex.h"
#include "RenderPass.h"
#include "Image.h"

namespace gfx
{
	// Don't initialize texture
	Texture::Texture(const GraphicsDevice& gfx)
		: m_tag("?")
	{

	}

	// Don't initialize texture
	Texture::Texture(const GraphicsDevice& gfx, UINT mipCount)
		: m_tag("?"), m_mipCount(mipCount)
	{

	}

	// Create descriptors and pTexture
	Texture::Texture(const GraphicsDevice& gfx, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc)
		: m_tag("?")
	{
		THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(
			&textureDesc, NULL, &m_pTexture
		));

		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(
			m_pTexture.Get(), &srvDesc, &m_pShaderResourceView
		));
	}

	void Texture::Release()
	{
		for (int i = 0, ct = m_pUAV.size(); i < ct; ++i)
		{
			m_pUAV[i].Reset();
		}
		m_pShaderResourceView.Reset();
		m_pTexture.Reset();
	}

	// Load image from file and start off with default 2D texture settings
	// This should be used for 99% of model-related textures
	Texture::Texture(const GraphicsDevice& gfx, const std::string& path)
		: m_tag(path)
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
			&textureDesc, nullptr, &m_pTexture
		));

		// Write image data into top mip level
		gfx.GetContext()->UpdateSubresource(
			m_pTexture.Get(), 0u, nullptr, image.GetData(), image.GetPitch(), 0u
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1; // use all mips

		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(
			m_pTexture.Get(), &srvDesc, &m_pShaderResourceView
		));

		gfx.GetContext()->GenerateMips(m_pShaderResourceView.Get());
	}

	// Load image from file and use custom descriptors
	// Use this in special cases
	Texture::Texture(const GraphicsDevice& gfx, const std::string& _path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc)
		: m_tag(_path)
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
				&textureDesc, nullptr, &m_pTexture
			));

			// Write image data into top mip level
			gfx.GetContext()->UpdateSubresource(
				m_pTexture.Get(), 0u, nullptr, image.GetData(), image.GetPitch(), 0u
			);

			gfx.GetContext()->GenerateMips(m_pShaderResourceView.Get());
		}
		else
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = image.GetData();
			sd.SysMemPitch = image.GetPitch(); // distance in bytes between rows - keep in mind padding!

			THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(
				&textureDesc, &sd, &m_pTexture
			));
		}

		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(
			m_pTexture.Get(), &srvDesc, &m_pShaderResourceView
		));
	}

	void Texture::BindCS(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::CS_SRV, slot))
		{
			gfx.GetContext()->CSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf());
		}
	}

	void Texture::UnbindCS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::CS_SRV, slot);
		gfx.GetContext()->CSSetShaderResources(slot, 1u, nullptr);
	}

	void Texture::BindVS(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::VS_SRV, slot))
		{
			gfx.GetContext()->VSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf());
		}
	}

	void Texture::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::VS_SRV, slot);
		gfx.GetContext()->VSSetShaderResources(slot, 1u, nullptr);
	}

	void Texture::BindPS(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_SRV, slot))
		{
			gfx.GetContext()->PSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf());
		}
	}

	void Texture::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_SRV, slot);
		gfx.GetContext()->PSSetShaderResources(slot, 1u, nullptr);
	}

	const ComPtr<ID3D11UnorderedAccessView> Texture::GetUAV(UINT mipSlice) const
	{
		return m_pUAV.size() > mipSlice ? m_pUAV[mipSlice] : nullptr;
	}

	const Texture::TextureDimension Texture::GetDimension() const
	{
		return m_dimension;
	}

	const ComPtr<ID3D11ShaderResourceView> Texture::GetSRV() const
	{
		return m_pShaderResourceView;
	}

	std::shared_ptr<Bindable> Texture::Resolve(const GraphicsDevice& gfx, const std::string& path)
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