#include "RenderTarget.h"
#include "GraphicsThrowMacros.h"
#include "DepthStencil.h"

namespace wrl = Microsoft::WRL;

RenderTarget::RenderTarget(Graphics& gfx)
	: RenderTexture::RenderTexture(gfx)
{
}

bool RenderTarget::Init(ID3D11Device * pDevice, int textureWidth, int textureHeight)
{
	if (RenderTexture::Init(pDevice, textureWidth, textureHeight))
	{
		viewport.Width = (FLOAT)textureWidth;
		viewport.Height = (FLOAT)textureHeight;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		return true;
	}
	return false;
}

void RenderTarget::BindAsTexture(Graphics& gfx, UINT slot) const
{
	GetContext(gfx)->PSSetShaderResources(slot, 1, pTextureView.GetAddressOf());
}

void RenderTarget::BindAsTarget(Graphics& gfx) const
{
	GetContext(gfx)->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
	GetContext(gfx)->RSSetViewports(1u, &viewport);
}

void RenderTarget::BindAsTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView) const
{
	GetContext(gfx)->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
	GetContext(gfx)->RSSetViewports(1u, &viewport);
}

void RenderTarget::SetRenderTarget(ID3D11DeviceContext* deviceContext, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView)
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