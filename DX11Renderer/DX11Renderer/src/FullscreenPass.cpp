#include "pch.h"
#include "FullscreenPass.h"
#include "RasterizerState.h"
#include "Binding.h"
#include "Bindable.h"
#include "Sampler.h"
#include "Texture.h"
#include "RawBufferData.h"
#include "VertexLayout.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Topology.h"
#include "InputLayout.h"
#include "VertexBufferWrapper.h"
#include "IndexBuffer.h"
#include "DepthStencilState.h"

namespace gfx
{
	FullscreenPass::FullscreenPass(const GraphicsDevice& gfx, const RenderPassType renderPassType, const char* pixelShader)
		: RenderPass(renderPassType)
	{
		std::string vertexShaderName("Assets\\Built\\Shaders\\FullscreenVS.cso");
		auto vs = VertexShader::Resolve(gfx, vertexShaderName.c_str());

		SetupFullscreenQuadBindings(gfx, vertexShaderName, *vs.get());

		AddBinding(std::move(vs))
			.SetupVSBinding(0u);
		AddBinding(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))
			.SetupIABinding();
		AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK))
			.SetupRSBinding();

		// Setup depth stencil state
		// todo: make this an option in DepthStencilState?
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
		dsDesc.DepthEnable = FALSE;
		//dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		dsDesc.StencilEnable = FALSE;
		AddBinding(std::move(std::make_shared<DepthStencilState>(gfx, dsDesc)))
			.SetupOMBinding();

		AddBinding(Sampler::Resolve(gfx, D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP))
			.SetupPSBinding(0u);

		AddBinding(PixelShader::Resolve(gfx, pixelShader))
			.SetupPSBinding(0u);
	}

	void FullscreenPass::Execute(const GraphicsDevice& gfx) const
	{
		RenderPass::Execute(gfx);

		m_pVertexBufferWrapper->BindIA(gfx, 0u);
		gfx.GetContext()->PSSetShaderResources(0u, 1u, m_pInputTexture->GetSRV().GetAddressOf());
		gfx.DrawIndexed(3u);
	}

	void FullscreenPass::SetInputTarget(std::shared_ptr<Texture> pInput)
	{
		m_pInputTexture = pInput;
	}

	void FullscreenPass::SetupFullscreenQuadBindings(const GraphicsDevice& gfx, std::string vertexShaderName, const VertexShader& vertexShader)
	{
		// Setup fullscreen geometry
		// Use 1 large triangle instead of 2 for better caching
		VertexLayout vertexLayout;
		vertexLayout.AppendVertexDesc<dx::XMFLOAT2>({ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });

		const auto pvsbc = vertexShader.GetBytecode();
		AddBinding(InputLayout::Resolve(gfx, vertexLayout, vertexShaderName, pvsbc))
			.SetupIABinding();

		RawBufferData vbuf(3u, vertexLayout.GetPerVertexStride(), vertexLayout.GetPerVertexPadding());
		vbuf.EmplaceBack(dx::XMFLOAT2{ 0, 0 });
		vbuf.EmplacePadding();
		vbuf.EmplaceBack(dx::XMFLOAT2{ 0, 2 });
		vbuf.EmplacePadding();
		vbuf.EmplaceBack(dx::XMFLOAT2{ 2, 0 });
		vbuf.EmplacePadding();
		m_pVertexBufferWrapper = std::make_unique<VertexBufferWrapper>(gfx, std::move(vbuf));

		std::vector<u32> indices = { 0u, 1u, 2u };
		AddBinding(IndexBuffer::Resolve(gfx, "$Blit", std::move(indices)))
			.SetupIABinding();
	}
}