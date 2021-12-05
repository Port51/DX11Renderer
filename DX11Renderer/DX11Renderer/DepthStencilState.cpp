#include "DepthStencilState.h"
#include "Graphics.h"

namespace Bind
{
	DepthStencilState::DepthStencilState(Graphics& gfx, Mode mode)
		: mode(mode)
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };

		// todo: set this somewhere else
		if (mode == Mode::Gbuffer)
		{
			dsDesc.StencilEnable = FALSE;
			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // allow less because maybe not everything will be in prepass
			// note - allow z-write for now
		}
		else if (mode == Mode::Write)
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

		gfx.GetDevice()->CreateDepthStencilState(&dsDesc, &pStencil);
	}

	DepthStencilState::DepthStencilState(Graphics & gfx, D3D11_DEPTH_STENCIL_DESC desc)
	{
		gfx.GetDevice()->CreateDepthStencilState(&desc, &pStencil);
	}

	void DepthStencilState::BindOM(Graphics& gfx, const RenderPass& renderPass)
	{
		gfx.GetContext()->OMSetDepthStencilState(pStencil.Get(), 0x01);
	}

	std::shared_ptr<DepthStencilState> DepthStencilState::Resolve(Graphics& gfx, Mode mode)
	{
		return Codex::Resolve<DepthStencilState>(gfx, GenerateUID(mode), mode);
	}

	std::string DepthStencilState::GenerateUID(Mode mode)
	{
		using namespace std::string_literals;
		const auto modeName = [mode]() {
			switch (mode)
			{
			case Mode::StencilOff:
				return "off"s;
			case Mode::Write:
				return "write"s;
			case Mode::Mask:
				return "mask"s;
			}
			return "ERROR"s;
		};
		return typeid(DepthStencilState).name() + "#"s + modeName();
	}

}