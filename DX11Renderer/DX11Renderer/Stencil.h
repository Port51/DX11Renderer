#pragma once
#include "Bindable.h"
#include "BindableCodex.h"
#include <d3d11.h>
#include <wrl.h>

namespace Bind
{
	class Stencil : public Bindable
	{
	public:
		enum class Mode
		{
			Off,
			Write,
			Mask
		};
		Stencil(Graphics& gfx, Mode mode)
			: mode(mode)
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };

			if (mode == Mode::Write)
			{
				dsDesc.StencilEnable = TRUE;
				dsDesc.StencilWriteMask = 0x01;

				dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
				dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

				dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
				dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
			}
			else if (mode == Mode::Mask)
			{
				dsDesc.DepthEnable = FALSE;
				dsDesc.StencilEnable = TRUE;
				dsDesc.StencilReadMask = 0x01;

				dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
				dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

				dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_EQUAL;
				dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
				dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
			}

			GetDevice(gfx)->CreateDepthStencilState(&dsDesc, &pStencil);
		}
		void Bind(Graphics& gfx, UINT slot) override
		{
			GetContext(gfx)->OMSetDepthStencilState(pStencil.Get(), 0x01);
		}
		static std::shared_ptr<Stencil> Resolve(Graphics& gfx, Mode mode)
		{
			return Codex::Resolve<Stencil>(gfx, mode);
		}
		static std::string GenerateUID(Mode mode)
		{
			using namespace std::string_literals;
			const auto modeName = [mode]() {
				switch (mode) {
				case Mode::Off:
					return "off"s;
				case Mode::Write:
					return "write"s;
				case Mode::Mask:
					return "mask"s;
				}
				return "ERROR"s;
			};
			return typeid(Stencil).name() + "#"s + modeName();
		}
		std::string GetUID() const override
		{
			return GenerateUID(mode);
		}
	private:
		Mode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencil;
	};
}