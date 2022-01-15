#include "pch.h"
#include "RenderTexture.h"
#include <assert.h>
#include "Graphics.h"

RenderTexture::RenderTexture(Graphics& gfx)
	: Texture::Texture(gfx)
{
}

//RenderTexture::~RenderTexture()
//{
//}

void RenderTexture::Init(ComPtr<ID3D11Device> device, UINT textureWidth, UINT textureHeight)
{
	// Setup the render target texture description.
	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));

	rtDesc.Width = textureWidth;
	rtDesc.Height = textureHeight;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.SampleDesc.Quality = 0;
	rtDesc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	rtDesc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE
	rtDesc.MiscFlags = 0;

	// Create the render target texture.
	THROW_IF_FAILED(device->CreateTexture2D(&rtDesc, NULL, &pTexture));

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc;
	ZeroMemory(&rtViewDesc, sizeof(rtViewDesc));
	rtViewDesc.Format = rtDesc.Format;
	rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	THROW_IF_FAILED(device->CreateRenderTargetView(pTexture.Get(), &rtViewDesc, &pRenderTargetView));

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	THROW_IF_FAILED(device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pShaderResourceView));

	if (rtDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		bool useCounter = false;
		if (!useCounter)
		{
			THROW_IF_FAILED(device->CreateUnorderedAccessView(pTexture.Get(), nullptr, &pUAV));
		}
		else
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			//uavDesc.Buffer.NumElements = numElements;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			THROW_IF_FAILED(device->CreateUnorderedAccessView(pTexture.Get(), &uavDesc, &pUAV));
		}
	}
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

ComPtr<ID3D11RenderTargetView> RenderTexture::GetView() const
{
	return pRenderTargetView;
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
