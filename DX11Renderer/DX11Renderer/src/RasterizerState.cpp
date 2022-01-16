#include "pch.h"
#include "RasterizerState.h"
#include "SharedCodex.h"
#include "Graphics.h"

namespace gfx
{
	RasterizerState::RasterizerState(Graphics& gfx, bool twoSided)
		:
		twoSided(twoSided)
	{
		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;

		THROW_IF_FAILED(gfx.GetDevice()->CreateRasterizerState(&rasterDesc, &pRasterizer));
	}

	void RasterizerState::BindRS(Graphics& gfx)
	{
		gfx.GetContext()->RSSetState(pRasterizer.Get());
	}

	std::shared_ptr<RasterizerState> RasterizerState::Resolve(Graphics& gfx, bool twoSided)
	{
		return Codex::Resolve<RasterizerState>(gfx, GenerateUID(twoSided), twoSided);
	}

	std::string RasterizerState::GenerateUID(bool twoSided)
	{
		using namespace std::string_literals;
		return typeid(RasterizerState).name() + "#"s + (twoSided ? "2s" : "1s");
	}
}