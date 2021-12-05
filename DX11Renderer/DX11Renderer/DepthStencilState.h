#pragma once
#include "Bindable.h"
#include "SharedCodex.h"
#include <d3d11.h>
#include <wrl.h>

namespace Bind
{
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
		void BindOM(Graphics& gfx, const RenderPass& renderPass) override;
		static std::shared_ptr<DepthStencilState> Resolve(Graphics& gfx, Mode mode);
	protected:
		static std::string GenerateUID(Mode mode);
	private:
		Mode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencil;
	};
}