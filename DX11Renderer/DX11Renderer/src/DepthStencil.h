#pragma once
#include "GraphicsDevice.h"

namespace gfx
{
	class DepthStencil
	{
		friend class RenderTexture;
		friend class GraphicsDevice;
	public:
		DepthStencil(const GraphicsDevice& gfx, const UINT width, const UINT height);
		virtual ~DepthStencil() = default;

	public:
		void BindAsDepthStencil(const GraphicsDevice& gfx) const;
		void Clear(const GraphicsDevice& gfx) const;

	private:
		ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;

	};
}