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
		RenderTexture(GraphicsDevice& gfx);
		RenderTexture(GraphicsDevice& gfx, UINT mipCount);
		RenderTexture(GraphicsDevice& gfx, DXGI_FORMAT format, UINT mipCount);

		virtual void Init(ComPtr<ID3D11Device> pDevice, UINT textureWidth, UINT textureHeight);
		void Shutdown();

		ComPtr<ID3D11RenderTargetView> GetView() const;

		void BindCS(GraphicsDevice& gfx, UINT slot) override;
		void BindVS(GraphicsDevice& gfx, UINT slot) override;
		void BindPS(GraphicsDevice& gfx, UINT slot) override;
		void BindAsTexture(GraphicsDevice& gfx, UINT slot) const;
		void BindAsTarget(GraphicsDevice& gfx) const;
		void BindAsTarget(GraphicsDevice& gfx, ComPtr<ID3D11DepthStencilView> pDepthStencilView) const;
		void SetRenderTarget(ID3D11DeviceContext* deviceContext, ComPtr<ID3D11DepthStencilView> depthStencilView);
		void ClearRenderTarget(ID3D11DeviceContext* deviceContext, float, float, float, float);
	protected:
		ComPtr<ID3D11RenderTargetView> pRenderTargetView;
		D3D11_VIEWPORT viewport;
		DXGI_FORMAT format;
	};
}