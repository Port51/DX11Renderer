#include "pch.h"
#include "SSAOPass.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "RenderTexture.h"
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
		CreateSubPass(SSAOSubpass::OcclusionSubpass);
		CreateSubPass(SSAOSubpass::HorizontalBlurSubpass);
		CreateSubPass(SSAOSubpass::VerticalBlurSubpass);

		m_pSSAOKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Assets\\Built\\Shaders\\SSAO.cso"));

		// Create random sample directions
		std::vector<dx::XMVECTOR> sampleOffsets;
		sampleOffsets.reserve(64);
		for (int i = 0; i < 64; ++i)
		{
			// Get random hemisphere direction
			sampleOffsets.emplace_back(dx::XMVector3Normalize(dx::XMVectorSet(
				rng.GetUniformFloat01() * 2.f - 1.f,
				rng.GetUniformFloat01() * 2.f - 1.f,
				rng.GetUniformFloat01(),
				1.f
			)));
		}
		m_pSampleOffsetSB = std::make_unique<StructuredBuffer<dx::XMVECTOR>>(gfx, D3D11_USAGE::D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 64u, sampleOffsets.data(), false);
	}

	void SSAOPass::SetupRenderPassDependencies(const GraphicsDevice & gfx, const RenderTexture & pGbuffer, const RenderTexture & pCameraColor)
	{
		GetSubPass(SSAOSubpass::OcclusionSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pGbuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, pCameraColor.GetSRV());
	}

	void SSAOPass::Execute(const GraphicsDevice & gfx) const
	{
		/*const RenderPass& pass = GetSubPass(RenderPassType::SSAORenderPass);
		pass.BindSharedResources(gfx);

		m_pSSAOKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

		pass.UnbindSharedResources(gfx);*/
	}

}