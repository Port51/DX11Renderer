#include "FullscreenPass.h"
#include "BindableInclude.h"
#include "RasterizerState.h"
#include "Binding.h"
#include "Bindable.h"
#include "Sampler.h"
#include "Texture.h"

namespace dx = DirectX;

FullscreenPass::FullscreenPass(Graphics& gfx, std::shared_ptr<Texture> pInput)
	: RenderPass()
{
	// setup fullscreen geometry
	VertexLayout lay;
	lay.Append(VertexLayout::Position2D);
	VertexBufferData bufFull(lay, (size_t)4);
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,-1 });
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,-1 });
	AddBinding(VertexBuffer::Resolve(gfx, "$Blit", std::move(bufFull)), 0u);

	std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
	AddBinding(IndexBuffer::Resolve(gfx, "$Blit", std::move(indices)), 0u);

	// setup other common fullscreen bindables
	auto vs = VertexShader::Resolve(gfx, "Assets\\Built\\Shaders\\FullscreenVS.cso");
	const auto pvsbc = vs->GetBytecode();

	AddBinding(InputLayout::Resolve(gfx, lay, pvsbc), 0u);
	AddBinding(std::move(vs), 0u);
	AddBinding(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST), 0u);
	AddBinding(Bind::RasterizerState::Resolve(gfx, false), 0u);

	AddBinding(Sampler::Resolve(gfx, D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP));
	AddBinding(pInput, 0u);
	//AddBinding(Texture::Resolve(gfx, "Models\\HeadTextures\\face_albedo_256.png"));

	AddBinding(PixelShader::Resolve(gfx, "Assets\\Built\\Shaders\\BlitPS.cso"), 0u);
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
