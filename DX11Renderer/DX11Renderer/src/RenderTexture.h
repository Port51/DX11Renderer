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
		RenderTexture(Graphics& gfx);
		//~RenderTexture();

		virtual void Init(ComPtr<ID3D11Device> pDevice, UINT textureWidth, UINT textureHeight);
		void Shutdown();

		ComPtr<ID3D11RenderTargetView> GetView() const;

		void BindCS(Graphics& gfx, UINT slot) override;
		void BindVS(Graphics& gfx, UINT slot) override;
		void BindPS(Graphics& gfx, UINT slot) override;

	protected:
		ComPtr<ID3D11RenderTargetView> pRenderTargetView;
	};
}