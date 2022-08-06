#include "pch.h"
#include "DepthStencilTarget.h"
#include "GraphicsDevice.h"

namespace gfx
{
	DepthStencilTarget::DepthStencilTarget(const GraphicsDevice& gfx, int width, int height)
	{
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
		THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(&descDepth, nullptr, &m_pDepthStencil));
		assert(m_pDepthStencil != NULL && "Depth stencil is null!");

		// Create DS View
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZERO_MEM(descDSV);
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0u;
		THROW_IF_FAILED(gfx.GetAdapter()->CreateDepthStencilView(m_pDepthStencil.Get(), &descDSV, &m_pDepthStencilView));
		assert(m_pDepthStencilView != NULL && "Depth stencil view is null!");

		// Create SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZERO_MEM(srvDesc);
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // DXGI_FORMAT_D24_UNORM_S8_UINT doesn't work
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(m_pDepthStencil.Get(), &srvDesc, &m_pShaderResourceView));
		assert(m_pShaderResourceView != NULL && "Depth SRV is null!");
	}

	DepthStencilTarget::~DepthStencilTarget()
	{
		Release();
	}

	void DepthStencilTarget::Release()
	{
		m_pDepthStencil.Reset();
		m_pDepthStencilView.Reset();
		m_pShaderResourceView.Reset();
		//SAFE_RELEASE(m_pDepthStencil);
		//SAFE_RELEASE(m_pDepthStencilView);
		//SAFE_RELEASE(m_pShaderResourceView);
	}

	void DepthStencilTarget::Clear(const GraphicsDevice& gfx) const
	{
		gfx.GetContext()->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.f, 0u);
	}

	const ComPtr<ID3D11DepthStencilView> DepthStencilTarget::GetView() const
	{
		return m_pDepthStencilView;
	}

	const ComPtr<ID3D11ShaderResourceView> DepthStencilTarget::GetSRV() const
	{
		return m_pShaderResourceView;
	}
}