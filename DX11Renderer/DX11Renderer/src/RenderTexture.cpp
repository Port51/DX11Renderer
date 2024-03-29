#include "pch.h"
#include "RenderTexture.h"
#include <assert.h>
#include "GraphicsDevice.h"

namespace gfx
{
	RenderTexture::RenderTexture(const GraphicsDevice& gfx)
		: Texture::Texture(gfx, 1u)
		, m_format(DXGI_FORMAT_R16G16B16A16_FLOAT)
	{}

	RenderTexture::RenderTexture(const GraphicsDevice& gfx, const UINT _mipCount)
		: Texture::Texture(gfx, _mipCount)
		, m_format(DXGI_FORMAT_R16G16B16A16_FLOAT)
	{}

	RenderTexture::RenderTexture(const GraphicsDevice& gfx, const DXGI_FORMAT _format)
		: Texture::Texture(gfx, 1u)
		, m_format(_format)
	{}

	RenderTexture::RenderTexture(const GraphicsDevice& gfx, const DXGI_FORMAT _format, const UINT _mipCount)
		: Texture::Texture(gfx, _mipCount)
		, m_format(_format)
	{}

	void RenderTexture::Init(ComPtr<ID3D11Device> device, const UINT textureWidth, const UINT textureHeight)
	{
		m_viewport.Width = (FLOAT)textureWidth;
		m_viewport.Height = (FLOAT)textureHeight;
		m_viewport.MinDepth = 0;
		m_viewport.MaxDepth = 1;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;

		// Setup the render target texture description.
		D3D11_TEXTURE2D_DESC rtDesc;
		ZERO_MEM(rtDesc);

		rtDesc.Width = textureWidth;
		rtDesc.Height = textureHeight;
		rtDesc.MipLevels = m_mipCount;
		rtDesc.ArraySize = 1;
		rtDesc.Format = m_format;
		rtDesc.SampleDesc.Count = 1;
		rtDesc.SampleDesc.Quality = 0;
		rtDesc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC
		rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		rtDesc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE
		rtDesc.MiscFlags = 0;

		// Create the render target texture.
		THROW_IF_FAILED(device->CreateTexture2D(&rtDesc, NULL, &m_pTexture));

		// Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc;
		ZERO_MEM(rtViewDesc);
		rtViewDesc.Format = rtDesc.Format;
		rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		THROW_IF_FAILED(device->CreateRenderTargetView(m_pTexture.Get(), &rtViewDesc, &m_pRenderTargetView));

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZERO_MEM(srvDesc);
		srvDesc.Format = rtDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = m_mipCount;

		// Create the shader resource view.
		THROW_IF_FAILED(device->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, &m_pShaderResourceView));

		if (rtDesc.BindFlags& D3D11_BIND_UNORDERED_ACCESS)
		{
			bool useCounter = false;
			m_pUAV.resize(m_mipCount);
			for (UINT mip = 0u; mip < m_mipCount; ++mip)
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

				THROW_IF_FAILED(device->CreateUnorderedAccessView(m_pTexture.Get(), &uavDesc, &m_pUAV[mip]));
			}
		}
	}

	void RenderTexture::Shutdown()
	{
		if (m_pShaderResourceView)
		{
			m_pShaderResourceView->Release();
		}

		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pTexture)
		{
			m_pTexture->Release();
		}

		return;
	}

	const ComPtr<ID3D11RenderTargetView> RenderTexture::GetRenderTargetView() const
	{
		return m_pRenderTargetView;
	}

	void RenderTexture::BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::CS_SRV, slot))
		{
			gfx.GetContext()->CSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf());
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void RenderTexture::UnbindCS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::CS_SRV, slot);
		gfx.GetContext()->CSSetShaderResources(slot, 1u, nullptr);
		REGISTER_GPU_CALL();
	}

	void RenderTexture::BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::VS_SRV, slot))
		{
			gfx.GetContext()->VSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf());
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void RenderTexture::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::VS_SRV, slot);
		gfx.GetContext()->VSSetShaderResources(slot, 1u, nullptr);
		REGISTER_GPU_CALL();
	}

	void RenderTexture::BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_SRV, slot))
		{
			gfx.GetContext()->PSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf());
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void RenderTexture::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_SRV, slot);
		gfx.GetContext()->PSSetShaderResources(slot, 1u, nullptr);
		REGISTER_GPU_CALL();
	}

	void RenderTexture::BindAsTexture(const GraphicsDevice& gfx, const slotUINT slot) const
	{
		gfx.GetContext()->PSSetShaderResources(slot, 1, m_pShaderResourceView.GetAddressOf());
		REGISTER_GPU_CALL();
	}

	void RenderTexture::BindAsTarget(const GraphicsDevice& gfx) const
	{
		gfx.GetContext()->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
		gfx.GetContext()->RSSetViewports(1u, &m_viewport);
		REGISTER_GPU_CALLS(2u);
	}

	void RenderTexture::BindAsTarget(const GraphicsDevice& gfx, const ComPtr<ID3D11DepthStencilView>& pDepthStencilView) const
	{
		gfx.GetContext()->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
		gfx.GetContext()->RSSetViewports(1u, &m_viewport);
		REGISTER_GPU_CALLS(2u);
	}

	void RenderTexture::SetRenderTarget(ID3D11DeviceContext* deviceContext, const ComPtr<ID3D11DepthStencilView>& pDepthStencilView)
	{
		// Bind the render target view and depth stencil buffer to the output render pipeline.
		deviceContext->OMSetRenderTargets(1u, m_pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
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
		deviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);

		return;
	}
}