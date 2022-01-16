#pragma once
#include "Graphics.h"

namespace gfx
{
	class DepthStencil
	{
		friend class RenderTarget;
		friend class Graphics;
	public:
		DepthStencil(Graphics& gfx, UINT width, UINT height);
		void BindAsDepthStencil(Graphics& gfx) const;
		void Clear(Graphics& gfx) const;
	private:
		ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	};
}