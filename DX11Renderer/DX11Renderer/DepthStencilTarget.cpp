#include "DepthStencilTarget.h"
#include "GraphicsThrowMacros.h"

DepthStencilTarget::DepthStencilTarget(Graphics& gfx, int width, int height)
{
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = (UINT)width;
	descDepth.Height = (UINT)height;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1u; // for AA
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	gfx.pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

	// create view of depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	gfx.pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDepthStencilView);
}

void DepthStencilTarget::Clear(Graphics& gfx)
{
	gfx.pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilTarget::GetView() const
{
	return pDepthStencilView;
}
