#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "SharedCodex.h"
#include "DX11Include.h"

namespace gfx
{
	class Graphics;

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
		DepthStencilState(Graphics& gfx, Mode mode);
		DepthStencilState(Graphics& gfx, D3D11_DEPTH_STENCIL_DESC desc);
		void BindOM(Graphics& gfx) override;
		static std::shared_ptr<DepthStencilState> Resolve(Graphics& gfx, Mode mode);
	protected:
		static std::string GenerateUID(Mode mode);
	private:
		Mode mode;
		ComPtr<ID3D11DepthStencilState> pStencil;
	};
}