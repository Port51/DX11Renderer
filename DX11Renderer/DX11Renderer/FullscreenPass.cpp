#include "FullscreenPass.h"
#include "BindableInclude.h"
#include "VertexBuffer.h"
#include "VertexBufferData.h"
#include "RasterizerState.h"


namespace Rendergraph
{
	namespace dx = DirectX;

	FullscreenPass::FullscreenPass(const std::string name, Graphics& gfx)
		:
	BindingPass(std::move(name))
	{
		// setup fullscreen geometry
		VertexLayout lay;
		lay.Append(VertexLayout::Position2D);
		VertexBufferData bufFull{ lay };
		bufFull.EmplaceBack(dx::XMFLOAT2{ -1,1 });
		bufFull.EmplaceBack(dx::XMFLOAT2{ 1,1 });
		bufFull.EmplaceBack(dx::XMFLOAT2{ -1,-1 });
		bufFull.EmplaceBack(dx::XMFLOAT2{ 1,-1 });
		AddBind(Bind::VertexBuffer::Resolve(gfx, "$Full", std::move(bufFull)));
		std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
		AddBind(Bind::IndexBuffer::Resolve(gfx, "$Full", std::move(indices)));
		// setup other common fullscreen bindables
		auto vs = Bind::VertexShader::Resolve(gfx, "Fullscreen_VS.cso");
		AddBind(Bind::InputLayout::Resolve(gfx, lay, vs->GetBytecode()));
		AddBind(std::move(vs));
		AddBind(Bind::Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddBind(Bind::RasterizerState::Resolve(gfx, false));
	}

	void FullscreenPass::Execute(Graphics& gfx) const
	{
		BindAll(gfx);
		gfx.DrawIndexed(6u);
	}
}