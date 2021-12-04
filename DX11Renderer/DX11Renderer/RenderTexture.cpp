#include "RenderTexture.h"
#include <assert.h>
#include "Graphics.h"
#include "GraphicsThrowMacros.h"

RenderTexture::RenderTexture(Graphics& gfx)
	: Texture::Texture(gfx)
{
}

//RenderTexture::~RenderTexture()
//{
//}

void RenderTexture::Init(ID3D11Device* device, int textureWidth, int textureHeight)
{
	SETUP_LOGGING_NOINFO(gfx);

	// Setup the render target texture description.
	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));

	rtDesc.Width = textureWidth;
	rtDesc.Height = textureHeight;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.Usage = D3D11_USAGE_DEFAULT;
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = 0;
	rtDesc.MiscFlags = 0;

	// Create the render target texture.
	GFX_THROW_NOINFO(device->CreateTexture2D(&rtDesc, NULL, &pTexture));

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc;
	ZeroMemory(&rtViewDesc, sizeof(rtViewDesc));
	rtViewDesc.Format = rtDesc.Format;
	rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	GFX_THROW_NOINFO(device->CreateRenderTargetView(pTexture.Get(), &rtViewDesc, &pRenderTargetView));

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	GFX_THROW_NOINFO(device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pShaderResourceView));
}

void RenderTexture::Shutdown()
{
	if (pShaderResourceView)
	{
		pShaderResourceView->Release();
	}

	if (pRenderTargetView)
	{
		pRenderTargetView->Release();
	}

	if (pTexture)
	{
		pTexture->Release();
	}

	return;
}

void RenderTexture::BindCS(Graphics& gfx, UINT slot)
{
	gfx.GetContext()->CSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
}

void RenderTexture::BindVS(Graphics& gfx, UINT slot)
{
	gfx.GetContext()->VSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
}

void RenderTexture::BindPS(Graphics& gfx, UINT slot)
{
	gfx.GetContext()->PSSetShaderResources(slot, 1u, pShaderResourceView.GetAddressOf());
}
