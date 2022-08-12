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
		DepthStencilTarget(const GraphicsDevice& gfx, int width, int height);
		virtual ~DepthStencilTarget();
		void Release();
	public:
		void Clear(const GraphicsDevice& gfx) const;
		const ComPtr<ID3D11DepthStencilView>& GetView() const;
		const ComPtr<ID3D11ShaderResourceView>& GetSRV() const;
	private:
		ComPtr<ID3D11Texture2D> m_pDepthStencil;
		ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
		ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
	};
}