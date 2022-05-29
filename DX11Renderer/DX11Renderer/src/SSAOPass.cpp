#include "pch.h"
#include "SSAOPass.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "RenderTexture.h"
#include "DepthStencilTarget.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Gaussian.h"
#include "ComputeShader.h"
#include "ComputeKernel.h"
#include "Binding.h"
#include "RandomGenerator.h"

namespace gfx
{

	SSAOPass::SSAOPass(const GraphicsDevice & gfx, RandomGenerator& rng)
		: RenderPass(RenderPassType::SSAORenderPass)
	{
		const UINT occlusionTextureWidth = gfx.GetScreenWidth();
		const UINT occlusionTextureHeight = gfx.GetScreenHeight();

		CreateSubPass(SSAOSubpass::OcclusionSubpass);
		CreateSubPass(SSAOSubpass::HorizontalBlurSubpass);
		CreateSubPass(SSAOSubpass::VerticalBlurSubpass);

		m_pOcclusionTexture0 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R8_UNORM);
		m_pOcclusionTexture0->Init(gfx.GetAdapter(), occlusionTextureWidth, occlusionTextureHeight);

		m_pOcclusionTexture1 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R8_UNORM);
		m_pOcclusionTexture1->Init(gfx.GetAdapter(), occlusionTextureWidth, occlusionTextureHeight);

		m_pOcclusionKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Assets\\Shaders\\SSAO.hlsl", "OcclusionPass"));
		m_pHorizontalBlurKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Assets\\Shaders\\SSAO.hlsl", "HorizontalBlurPass"));
		m_pVerticalBlurKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Assets\\Shaders\\SSAO.hlsl", "VerticalBlurPass"));

		// Create random sample directions
		std::vector<dx::XMVECTOR> sampleOffsets;
		sampleOffsets.reserve(SampleOffsetCount);
		for (int i = 0; i < SampleOffsetCount; ++i)
		{
			// Get random hemisphere direction
			sampleOffsets.emplace_back(dx::XMVector3Normalize(dx::XMVectorSet(
				rng.GetUniformFloat01() * 2.f - 1.f,
				rng.GetUniformFloat01() * 2.f - 1.f,
				rng.GetUniformFloat01(),
				0.f
			)));
		}
		m_pSampleOffsetSB = std::make_unique<StructuredBuffer<dx::XMVECTOR>>(gfx, D3D11_USAGE::D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, SampleOffsetCount, sampleOffsets.data(), false);
	}

	void SSAOPass::SetupRenderPassDependencies(const GraphicsDevice & gfx, const RenderTexture & pGbuffer, const RenderTexture& hiZBuffer, const Texture& noiseTexture)
	{
		GetSubPass(SSAOSubpass::OcclusionSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pGbuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, hiZBuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, m_pSampleOffsetSB->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, noiseTexture.GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pOcclusionTexture0->GetUAV());

		GetSubPass(SSAOSubpass::HorizontalBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, m_pOcclusionTexture0->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pOcclusionTexture1->GetUAV());

		GetSubPass(SSAOSubpass::VerticalBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, m_pOcclusionTexture1->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pOcclusionTexture0->GetUAV());
	}

	void SSAOPass::Execute(const GraphicsDevice & gfx) const
	{
		// Render occlusion
		{
			const RenderPass& pass = GetSubPass(SSAOSubpass::OcclusionSubpass);
			pass.BindSharedResources(gfx);

			m_pOcclusionKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

			pass.UnbindSharedResources(gfx);
		}

		// Horizontal blur
		{
			const RenderPass& pass = GetSubPass(SSAOSubpass::HorizontalBlurSubpass);
			pass.BindSharedResources(gfx);

			m_pHorizontalBlurKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

			pass.UnbindSharedResources(gfx);
		}

		// Vertical blur
		{
			const RenderPass& pass = GetSubPass(SSAOSubpass::VerticalBlurSubpass);
			pass.BindSharedResources(gfx);

			m_pVerticalBlurKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

			pass.UnbindSharedResources(gfx);
		}
	}

	const RenderTexture & SSAOPass::GetOcclusionTexture() const
	{
		return *m_pOcclusionTexture0;
	}

}