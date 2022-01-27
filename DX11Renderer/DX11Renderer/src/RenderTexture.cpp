#include "pch.h"
#include "RenderTexture.h"
#include <assert.h>
#include "Graphics.h"

// Dev notes:
// - One-line pathway for Texture loaded from file with dimensions, generated mipmaps/no mips. Always has SRV.
// - RT requires Init()
	// Common options:
		// - FORMAT, no mips, bind flags
		// - All descriptions done manually
	// - Add Viewport

namespace gfx
{
	RenderTexture::RenderTexture(Graphics& gfx)
		: Texture::Texture(gfx)
	{
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		mipCount = 1u;
	}

	RenderTexture::RenderTexture(Graphics & gfx, UINT _mipCount)
		: Texture::Texture(gfx)
	{
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		mipCount = _mipCount;
	}

	RenderTexture::RenderTexture(Graphics & gfx, DXGI_FORMAT _format, UINT _mipCount)
		: Texture::Texture(gfx)
	{
		format = _format;
		mipCount = _mipCount;
	}

	void RenderTexture::Init(ComPtr<ID3D11Device> device, UINT textureWidth, UINT textureHeight)
	{
		viewport.Width = (FLOAT)textureWidth;
		viewport.Height = (FLOAT)textureHeight;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		// Setup the render target texture description.
		D3D11_TEXTURE2D_DESC rtDesc;
		ZeroMemory(&rtDesc, sizeof(rtDesc));

		rtDesc.Width = textureWidth;
		rtDesc.Height = textureHeight;
		rtDesc.MipLevels = mipCount;
		rtDesc.ArraySize = 1;
		rtDesc.Format = format;
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
		srvDesc.Texture2D.MipLevels = mipCount;

		// Create the shader resource view.
		THROW_IF_FAILED(device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pShaderResourceView));

		if (rtDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			bool useCounter = false;
			pUAV.resize(mipCount);
			for (UINT mip = 0u; mip < mipCount; ++mip)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
				uavDesc.Format = rtDesc.Format;
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = mip;

				if (useCounter)
				{
					uavDesc.Buffer.FirstElement = 0;
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
				}

				THROW_IF_FAILED(device->CreateUnorderedAccessView(pTexture.Get(), &uavDesc, &pUAV[mip]));
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

	void RenderTexture::BindAsTexture(Graphics& gfx, UINT slot) const
	{
		gfx.GetContext()->PSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf());
	}

	void RenderTexture::BindAsTarget(Graphics& gfx) const
	{
		gfx.GetContext()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
		gfx.GetContext()->RSSetViewports(1u, &viewport);
	}

	void RenderTexture::BindAsTarget(Graphics& gfx, ComPtr<ID3D11DepthStencilView> pDepthStencilView) const
	{
		gfx.GetContext()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
		gfx.GetContext()->RSSetViewports(1u, &viewport);
	}

	void RenderTexture::SetRenderTarget(ID3D11DeviceContext* deviceContext, ComPtr<ID3D11DepthStencilView> pDepthStencilView)
	{
		// Bind the render target view and depth stencil buffer to the output render pipeline.
		deviceContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
	}

	void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* deviceContext,
		float red, float green, float blue, float alpha)
	{
		static float color[4];


		// Setup the color to clear the buffer to.
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		color[3] = alpha;

		// Clear the back buffer.
		deviceContext->ClearRenderTargetView(pRenderTargetView.Get(), color);

		return;
	}
}