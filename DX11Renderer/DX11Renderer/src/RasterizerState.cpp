#include "pch.h"
#include "RasterizerState.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	RasterizerState::RasterizerState(const GraphicsDevice& gfx, const D3D11_CULL_MODE cullMode)
		: m_cullMode(cullMode)
	{
		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = cullMode;

		if (cullMode == D3D11_CULL_FRONT)
		{
			rasterDesc.FrontCounterClockwise = true;
		}

		THROW_IF_FAILED(gfx.GetAdapter()->CreateRasterizerState(&rasterDesc, &m_pRasterizer));
	}

	void RasterizerState::Release()
	{
		m_pRasterizer.Reset();
	}

	void RasterizerState::BindRS(const GraphicsDevice& gfx, RenderState& renderState)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::RS_State, 0u))
		{
			gfx.GetContext()->RSSetState(m_pRasterizer.Get());
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void RasterizerState::UnbindRS(const GraphicsDevice & gfx, RenderState & renderState)
	{
		renderState.ClearBinding(RenderBindingType::RS_State, 0u);
		gfx.GetContext()->RSSetState(nullptr);
		REGISTER_GPU_CALL();
	}

	std::shared_ptr<RasterizerState> RasterizerState::Resolve(const GraphicsDevice& gfx, const D3D11_CULL_MODE cullMode)
	{
		return std::move(Codex::Resolve<RasterizerState>(gfx, GenerateUID(cullMode), cullMode));
	}

	const D3D11_CULL_MODE RasterizerState::GetCullModeFromMaterialString(const std::string name)
	{
		if (name == "Front")
		{
			return D3D11_CULL_FRONT;
		}
		else if (name == "Back")
		{
			return D3D11_CULL_BACK;
		}
		else if (name == "None")
		{
			return D3D11_CULL_NONE;
		}
		else
		{
			THROW("Unhandled rasterizer state " + name);
			return D3D11_CULL_BACK;
		}
	}

	std::string RasterizerState::GenerateUID(const D3D11_CULL_MODE cullMode)
	{
		using namespace std::string_literals;
		return typeid(RasterizerState).name() + "#"s + std::to_string((int)cullMode);
	}
}