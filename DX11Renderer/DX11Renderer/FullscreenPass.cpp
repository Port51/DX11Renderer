#include "FullscreenPass.h"
#include "BindableInclude.h"
#include "RasterizerState.h"
#include "Binding.h"
#include "Bindable.h"
#include "Sampler.h"
#include "Texture.h"
#include "VertexBufferData.h"

namespace dx = DirectX;

FullscreenPass::FullscreenPass(Graphics& gfx, std::shared_ptr<Texture> pInputTexture)
	: RenderPass()
{
	// setup fullscreen geometry
	VertexLayout vertexLayout;
	vertexLayout.AppendVertexDesc<dx::XMFLOAT2>({ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });

	VertexBufferData bufFull((size_t)4, vertexLayout.GetPerVertexStride(), vertexLayout.GetPerVertexPadding());
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,1 });
	bufFull.EmplacePadding();
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,1 });
	bufFull.EmplacePadding();
	bufFull.EmplaceBack(dx::XMFLOAT2{ -1,-1 });
	bufFull.EmplacePadding();
	bufFull.EmplaceBack(dx::XMFLOAT2{ 1,-1 });
	bufFull.EmplacePadding();
	pVertexBufferWrapper = std::make_unique<VertexBufferWrapper>(gfx, "$Blit", std::move(bufFull));

	std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
	AddBinding(IndexBuffer::Resolve(gfx, "$Blit", std::move(indices)))
		.SetupIABinding();

	// setup other common fullscreen bindables
	auto vs = VertexShader::Resolve(gfx, "Assets\\Built\\Shaders\\FullscreenVS.cso");
	const auto pvsbc = vs->GetBytecode();

	AddBinding(InputLayout::Resolve(gfx, vertexLayout, pvsbc))
		.SetupIABinding();
	AddBinding(std::move(vs))
		.SetupVSBinding(0u);
	AddBinding(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))
		.SetupIABinding();
	AddBinding(Bind::RasterizerState::Resolve(gfx, false))
		.SetupRSBinding();

	AddBinding(Sampler::Resolve(gfx, D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP))
		.SetupPSBinding(0u);
	AddBinding(pInputTexture)
		.SetupPSBinding(0u);
	//AddBinding(Texture::Resolve(gfx, "Models\\HeadTextures\\face_albedo_256.png"));

	AddBinding(PixelShader::Resolve(gfx, "Assets\\Built\\Shaders\\BlitPS.cso"))
		.SetupPSBinding(0u);
}

void FullscreenPass::Execute(Graphics& gfx) const
{
	for (auto& binding : bindings)
	{
		binding.Bind(gfx);
	}
	pVertexBufferWrapper->BindIA(gfx, 0u);
	gfx.DrawIndexed(6u);
}

Binding& FullscreenPass::AddBinding(std::shared_ptr<Bindable> pBindable)
{
	bindings.push_back(Binding(std::move(pBindable)));
	return bindings[bindings.size() - 1];
}

Binding& FullscreenPass::AddBinding(Binding pBinding)
{
	bindings.push_back(std::move(pBinding));
	return bindings[bindings.size() - 1];
}
