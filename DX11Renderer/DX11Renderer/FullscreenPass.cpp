#include "FullscreenPass.h"
#include "BindableInclude.h"
#include "RasterizerState.h"

namespace dx = DirectX;

FullscreenPass::FullscreenPass(const std::string name, Graphics& gfx)
	: BindingPass(std::move(name))
{
	// setup fullscreen geometry
	VertexLayout lay;
	lay.Append(VertexLayout::Position2D);
	VertexBufferData bufFull{ lay };
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,-1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,-1 });
	AddBind(VertexBuffer::Resolve(gfx, "$Blit", std::move(bufFull)));

	std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
	AddBind(IndexBuffer::Resolve(gfx, "$Blit", std::move(indices)));

	// setup other common fullscreen bindables
	auto vs = VertexShader::Resolve(gfx, "FullscreenVS.cso");
	const auto pvsbc = vs->GetBytecode();

	AddBind(InputLayout::Resolve(gfx, lay, pvsbc));
	AddBind(std::move(vs));
	AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	AddBind(Bind::RasterizerState::Resolve(gfx, false));

	AddBind(PixelShader::Resolve(gfx, "BlitPS.cso"));
}

void FullscreenPass::Execute(Graphics& gfx) const
{
	BindAll(gfx);
	gfx.DrawIndexed(6u);
}