#include "pch.h"
#include "RasterizerState.h"
#include "SharedCodex.h"
#include "Graphics.h"

namespace gfx
{
	RasterizerState::RasterizerState(Graphics& gfx, D3D11_CULL_MODE cullMode)
		: cullMode(cullMode)
	{
		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = cullMode;

		if (cullMode == D3D11_CULL_FRONT)
		{
			rasterDesc.FrontCounterClockwise = true;
		}

		THROW_IF_FAILED(gfx.GetDevice()->CreateRasterizerState(&rasterDesc, &pRasterizer));
	}

	void RasterizerState::BindRS(Graphics& gfx)
	{
		gfx.GetContext()->RSSetState(pRasterizer.Get());
	}

	std::shared_ptr<RasterizerState> RasterizerState::Resolve(Graphics& gfx, D3D11_CULL_MODE cullMode)
	{
		return Codex::Resolve<RasterizerState>(gfx, GenerateUID(cullMode), cullMode);
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