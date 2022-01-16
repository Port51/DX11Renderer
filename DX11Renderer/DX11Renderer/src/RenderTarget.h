#pragma once
#include "Graphics.h"
#include "RenderTexture.h"

namespace gfx
{
	class DepthStencil;

	class RenderTarget : public RenderTexture
	{
	public:
		RenderTarget(Graphics& gfx);
		void Init(ComPtr<ID3D11Device> pDevice, UINT textureWidth, UINT textureHeight) override;
		void BindAsTexture(Graphics& gfx, UINT slot) const;
		void BindAsTarget(Graphics& gfx) const;
		void BindAsTarget(Graphics& gfx, ComPtr<ID3D11DepthStencilView> pDepthStencilView) const;
		void SetRenderTarget(ID3D11DeviceContext* deviceContext, ComPtr<ID3D11DepthStencilView> depthStencilView);
		void ClearRenderTarget(ID3D11DeviceContext* deviceContext, float, float, float, float);
	private:
		D3D11_VIEWPORT viewport;
	};
}