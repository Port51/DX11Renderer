#pragma once
#include "Bindable.h"
#include "BindableCodex.h"
#include <d3d11.h>
#include <wrl.h>

namespace Bind
{
	class DepthStencilState : public Bindable
	{
	public:
		enum class Mode
		{
			Off,
			Write,
			Mask
		};
		DepthStencilState(Graphics& gfx, Mode mode);
		DepthStencilState(Graphics& gfx, D3D11_DEPTH_STENCIL_DESC desc);
		void BindOM(Graphics& gfx) override;
		static std::shared_ptr<DepthStencilState> Resolve(Graphics& gfx, Mode mode);
		static std::string GenerateUID(Mode mode);
		std::string GetUID() const override;
	private:
		Mode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencil;
	};
}