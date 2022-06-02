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
		DepthStencilState(const GraphicsDevice& gfx, const Mode mode);
		DepthStencilState(const GraphicsDevice& gfx, const D3D11_DEPTH_STENCIL_DESC desc);
		virtual void Release() override;
		void BindOM(const GraphicsDevice& gfx, RenderState& renderState) override;
		void UnbindOM(const GraphicsDevice& gfx, RenderState& renderState) override;
		static std::shared_ptr<DepthStencilState> Resolve(const GraphicsDevice& gfx, const Mode mode);
	protected:
		static std::string GenerateUID(const Mode mode);
	private:
		const Mode m_mode;
		ComPtr<ID3D11DepthStencilState> m_pStencil;
	};
}