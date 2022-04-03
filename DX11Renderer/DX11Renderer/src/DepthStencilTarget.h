#pragma once
#include "CommonHeader.h"

struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace gfx
{
	class GraphicsDevice;

	class DepthStencilTarget
	{
	public:
		DepthStencilTarget(GraphicsDevice& gfx, int width, int height);
		virtual ~DepthStencilTarget();
	public:
		void Clear(GraphicsDevice& gfx);
		ComPtr<ID3D11DepthStencilView> GetView() const;
		ComPtr<ID3D11ShaderResourceView> GetSRV() const;
	private:
		ComPtr<ID3D11Texture2D> m_pDepthStencil;
		ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
		ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
	};
}