#pragma once
#include "GraphicsDevice.h"

namespace gfx
{
	class DepthStencil
	{
		friend class RenderTexture;
		friend class GraphicsDevice;
	public:
		DepthStencil(GraphicsDevice& gfx, UINT width, UINT height);
		virtual ~DepthStencil() = default;
	public:
		void BindAsDepthStencil(GraphicsDevice& gfx) const;
		void Clear(GraphicsDevice& gfx) const;
	private:
		ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	};
}