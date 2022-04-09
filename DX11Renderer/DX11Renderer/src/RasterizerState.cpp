#include "pch.h"
#include "RasterizerState.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	RasterizerState::RasterizerState(const GraphicsDevice& gfx, D3D11_CULL_MODE cullMode)
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

	void RasterizerState::BindRS(const GraphicsDevice& gfx)
	{
		gfx.GetContext()->RSSetState(m_pRasterizer.Get());
	}

	std::shared_ptr<RasterizerState> RasterizerState::Resolve(const GraphicsDevice& gfx, D3D11_CULL_MODE cullMode)
	{
		return std::move(Codex::Resolve<RasterizerState>(gfx, GenerateUID(cullMode), cullMode));
	}

	D3D11_CULL_MODE RasterizerState::GetCullModeFromMaterialString(std::string name)
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
			throw std::runtime_error("Unhandled rasterizer state " + name);
			return D3D11_CULL_BACK;
		}
	}

	std::string RasterizerState::GenerateUID(D3D11_CULL_MODE cullMode)
	{
		using namespace std::string_literals;
		return typeid(RasterizerState).name() + "#"s + std::to_string((int)cullMode);
	}
}