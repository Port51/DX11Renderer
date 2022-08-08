#include "pch.h"
#include "DepthStencil.h"
#include "Config.h"
#include "GraphicsDevice.h"

namespace gfx
{
	DepthStencil::DepthStencil(const GraphicsDevice& gfx, const UINT width, const UINT height)
	{
		// create depth stensil texture
		ComPtr<ID3D11Texture2D> pDepthStencil;
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1u;
		descDepth.ArraySize = 1u;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = (UINT)Config::MsaaSamples;
		descDepth.SampleDesc.Quality = (UINT)Config::MsaaQuality;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		THROW_IF_FAILED(gfx.GetAdapter()->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

		// create target view of depth stensil texture
		THROW_IF_FAILED(gfx.GetAdapter()->CreateDepthStencilView(
			pDepthStencil.Get(), nullptr, &m_pDepthStencilView
		));
	}

	DepthStencil::~DepthStencil()
	{}

	void DepthStencil::BindAsDepthStencil(const GraphicsDevice& gfx) const
	{
		gfx.GetContext()->OMSetRenderTargets(0, nullptr, m_pDepthStencilView.Get());
	}

	void DepthStencil::Clear(const GraphicsDevice& gfx) const
	{
		gfx.GetContext()->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	}
}