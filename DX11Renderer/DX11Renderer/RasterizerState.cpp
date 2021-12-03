#include "RasterizerState.h"
#include "GraphicsThrowMacros.h"
#include "SharedCodex.h"

namespace Bind
{
	RasterizerState::RasterizerState(Graphics& gfx, bool twoSided)
		:
		twoSided(twoSided)
	{
		SETUP_LOGGING(gfx);

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;

		GFX_THROW_INFO(GetDevice(gfx)->CreateRasterizerState(&rasterDesc, &pRasterizer));
	}

	void RasterizerState::BindRS(Graphics& gfx)
	{
		GetContext(gfx)->RSSetState(pRasterizer.Get());
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