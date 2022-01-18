#pragma once
#include "CommonHeader.h"

struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace gfx
{
	class Graphics;

	class DepthStencilTarget
	{
	public:
		DepthStencilTarget(Graphics& gfx, int width, int height);
		virtual ~DepthStencilTarget();
	public:
		void Clear(Graphics& gfx);
		ComPtr<ID3D11DepthStencilView> GetView() const;
		ComPtr<ID3D11ShaderResourceView> GetSRV() const;
	private:
		ComPtr<ID3D11Texture2D> pDepthStencil;
		ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	};
}