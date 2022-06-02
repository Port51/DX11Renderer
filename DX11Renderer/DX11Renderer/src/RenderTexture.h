#pragma once
#include "Texture.h"
#include "DX11Include.h"
#include "CommonHeader.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;

namespace gfx
{

	class RenderTexture : public Texture
	{
	public:
		RenderTexture(const GraphicsDevice& gfx);
		RenderTexture(const GraphicsDevice& gfx, const UINT mipCount);
		RenderTexture(const GraphicsDevice& gfx, const DXGI_FORMAT format);
		RenderTexture(const GraphicsDevice& gfx, const DXGI_FORMAT format, const UINT mipCount);

		virtual void Init(ComPtr<ID3D11Device> pDevice, const UINT textureWidth, const UINT textureHeight);
		void Shutdown();

		const ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const;

		void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindAsTexture(const GraphicsDevice& gfx, const slotUINT slot) const;
		void BindAsTarget(const GraphicsDevice& gfx) const;
		void BindAsTarget(const GraphicsDevice& gfx, ComPtr<ID3D11DepthStencilView> pDepthStencilView) const;
		void SetRenderTarget(ID3D11DeviceContext* deviceContext, ComPtr<ID3D11DepthStencilView> depthStencilView);
		void ClearRenderTarget(ID3D11DeviceContext* deviceContext, float, float, float, float);
	protected:
		ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
		D3D11_VIEWPORT m_viewport;
		DXGI_FORMAT m_format;
	};
}