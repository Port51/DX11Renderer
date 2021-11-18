#include "FullscreenPass.h"
#include "BindableInclude.h"
#include "RasterizerState.h"
#include "Binding.h"
#include "Bindable.h"

namespace dx = DirectX;

FullscreenPass::FullscreenPass(Graphics& gfx)
	: RenderPass()
{
	// setup fullscreen geometry
	VertexLayout lay;
	lay.Append(VertexLayout::Position2D);
	VertexBufferData bufFull{ lay };
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,-1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,-1 });
	AddBinding(VertexBuffer::Resolve(gfx, "$Blit", std::move(bufFull)), 0u);

	std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
	AddBinding(IndexBuffer::Resolve(gfx, "$Blit", std::move(indices)), 0u);

	// setup other common fullscreen bindables
	auto vs = VertexShader::Resolve(gfx, "Shaders\\Built\\FullscreenVS.cso");
	const auto pvsbc = vs->GetBytecode();

	AddBinding(InputLayout::Resolve(gfx, lay, pvsbc), 0u);
	AddBinding(std::move(vs), 0u);
	AddBinding(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST), 0u);
	AddBinding(Bind::RasterizerState::Resolve(gfx, false), 0u);

	AddBinding(PixelShader::Resolve(gfx, "Shaders\\Built\\BlitPS.cso"), 0u);
}

void FullscreenPass::Execute(Graphics& gfx) const
{
	for (auto& binding : bindings)
	{
		binding.Bind(gfx);
	}
	gfx.DrawIndexed(6u);
}

void FullscreenPass::AddBinding(std::shared_ptr<Bindable> pBindable, UINT slot)
{
	bindings.push_back(Binding(std::move(pBindable), slot));
}

void FullscreenPass::AddBinding(Binding pBinding)
{
	bindings.push_back(std::move(pBinding));
}
