#include "pch.h"
#include "DepthStencilState.h"
#include "GraphicsDevice.h"

namespace gfx
{
	DepthStencilState::DepthStencilState(const GraphicsDevice& gfx, const Mode mode)
		: m_mode(mode)
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };

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

		gfx.GetAdapter()->CreateDepthStencilState(&dsDesc, &m_pStencil);
	}

	DepthStencilState::DepthStencilState(const GraphicsDevice& gfx, const D3D11_DEPTH_STENCIL_DESC desc)
		: m_mode(Mode::StencilOff)
	{
		gfx.GetAdapter()->CreateDepthStencilState(&desc, &m_pStencil);
	}

	void DepthStencilState::Release()
	{
		m_pStencil.Reset();
	}

	void DepthStencilState::BindOM(const GraphicsDevice& gfx, RenderState& renderState)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::OM_DepthStencilState, 0u))
		{
			gfx.GetContext()->OMSetDepthStencilState(m_pStencil.Get(), 0x01);
		}
	}

	void DepthStencilState::UnbindOM(const GraphicsDevice & gfx, RenderState & renderState)
	{
		renderState.ClearBinding(RenderBindingType::OM_DepthStencilState, 0u);
		gfx.GetContext()->OMSetDepthStencilState(nullptr, 0x01);
	}

	std::shared_ptr<DepthStencilState> DepthStencilState::Resolve(const GraphicsDevice& gfx, const Mode mode)
	{
		return std::move(Codex::Resolve<DepthStencilState>(gfx, GenerateUID(mode), mode));
	}

	std::string DepthStencilState::GenerateUID(const Mode mode)
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