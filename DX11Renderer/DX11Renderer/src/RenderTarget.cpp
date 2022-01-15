#include "pch.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

namespace wrl = Microsoft::WRL;

RenderTarget::RenderTarget(Graphics& gfx)
	: RenderTexture::RenderTexture(gfx)
{
}

void RenderTarget::Init(ComPtr<ID3D11Device> pDevice, UINT textureWidth, UINT textureHeight)
{
	RenderTexture::Init(pDevice, textureWidth, textureHeight);

	viewport.Width = (FLOAT)textureWidth;
	viewport.Height = (FLOAT)textureHeight;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
}

void RenderTarget::BindAsTexture(Graphics& gfx, UINT slot) const
{
	gfx.GetContext()->PSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf());
}

void RenderTarget::BindAsTarget(Graphics& gfx) const
{
	gfx.GetContext()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
	gfx.GetContext()->RSSetViewports(1u, &viewport);
}

void RenderTarget::BindAsTarget(Graphics& gfx, ComPtr<ID3D11DepthStencilView> pDepthStencilView) const
{
	gfx.GetContext()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
	gfx.GetContext()->RSSetViewports(1u, &viewport);
}

void RenderTarget::SetRenderTarget(ID3D11DeviceContext* deviceContext, ComPtr<ID3D11DepthStencilView> pDepthStencilView)
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
}

void RenderTarget::ClearRenderTarget(ID3D11DeviceContext* deviceContext,
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

	return;
}