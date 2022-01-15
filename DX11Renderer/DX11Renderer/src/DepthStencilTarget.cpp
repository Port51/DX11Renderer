#include "DepthStencilTarget.h"
#include "GraphicsThrowMacros.h"

DepthStencilTarget::DepthStencilTarget(Graphics& gfx, int width, int height)
{
	SETUP_LOGGING_NOINFO(gfx);

	// Notes on formats:
	// To create SRV, must use a typeless format for texture
	// Use DXGI_FORMAT_R32_TYPELESS for 32 bit depth
	// Use DXGI_FORMAT_R24G8_TYPELESS for 24 bit depth + 8 bit stencil

	// Create texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = (UINT)width;
	descDepth.Height = (UINT)height;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
	descDepth.SampleDesc.Count = 1u; // for AA
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_NOINFO(gfx.GetDevice()->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));
	assert(pDepthStencil != NULL && "Depth stencil is null!");

	// Create DS View
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_NOINFO(gfx.GetDevice()->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDepthStencilView));
	assert(pDepthStencilView != NULL && "Depth stencil view is null!");

	// Create SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // DXGI_FORMAT_D24_UNORM_S8_UINT doesn't work
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GFX_THROW_NOINFO(gfx.GetDevice()->CreateShaderResourceView(pDepthStencil.Get(), &srvDesc, &pShaderResourceView));
	assert(pShaderResourceView != NULL && "Depth SRV is null!");
}

void DepthStencilTarget::Clear(Graphics& gfx)
{
	gfx.GetContext()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilTarget::GetView() const
{
	return pDepthStencilView;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DepthStencilTarget::GetSRV() const
{
	return pShaderResourceView;
}
