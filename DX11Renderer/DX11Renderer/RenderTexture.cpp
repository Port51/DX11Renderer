#include "RenderTexture.h"
#include <assert.h>

RenderTexture::RenderTexture(Graphics& gfx)
	: Texture::Texture(gfx)
{
}

//RenderTexture::~RenderTexture()
//{
//}

bool RenderTexture::Init(ID3D11Device* device, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &pRenderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(pRenderTargetTexture.Get(), &renderTargetViewDesc, &pRenderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(pRenderTargetTexture.Get(), &shaderResourceViewDesc, &pRenderTextureView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void RenderTexture::Shutdown()
{
	if (pRenderTextureView)
	{
		pRenderTextureView->Release();
	}

	if (pRenderTargetView)
	{
		pRenderTargetView->Release();
	}

	if (pRenderTargetTexture)
	{
		pRenderTargetTexture->Release();
	}

	return;
}

void RenderTexture::Bind(Graphics& gfx, UINT slot)
{
	GetContext(gfx)->PSSetShaderResources(slot, 1u, pRenderTextureView.GetAddressOf());
}

void RenderTexture::SetRenderTarget(ID3D11DeviceContext* deviceContext, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView)
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), depthStencilView.Get());
}

void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* deviceContext, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView,
	float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(pRenderTargetView.Get(), color);

	// Clear the depth buffer.
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}