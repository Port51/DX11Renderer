#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "SharedCodex.h"
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	class DepthStencilState : public Bindable
	{
	public:
		enum class Mode
		{
			StencilOff,
			Gbuffer,
			Write,
			Mask
		};
		DepthStencilState(GraphicsDevice& gfx, Mode mode);
		DepthStencilState(GraphicsDevice& gfx, D3D11_DEPTH_STENCIL_DESC desc);
		void BindOM(GraphicsDevice& gfx) override;
		static std::shared_ptr<DepthStencilState> Resolve(GraphicsDevice& gfx, Mode mode);
	protected:
		static std::string GenerateUID(Mode mode);
	private:
		Mode m_mode;
		ComPtr<ID3D11DepthStencilState> m_pStencil;
	};
}