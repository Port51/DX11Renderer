#pragma once
#include "pch.h"
#include "SkyboxPass.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "RenderTexture.h"
#include "GraphicsDevice.h"

namespace gfx
{
	SkyboxPass::SkyboxPass(const GraphicsDevice& gfx)
		: RenderPass(RenderPassType::SkyboxRenderPass)
	{
		const char* computeShaderPath = "Skybox.hlsl";
		m_pSkyboxKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath));
	}

	void SkyboxPass::Execute(const GraphicsDevice & gfx, RenderState & renderState) const
	{
		gfx.GetRenderStats().StartTaskTimer(GetName());
		const UINT screenWidth = gfx.GetScreenWidth();
		const UINT screenHeight = gfx.GetScreenHeight();

		BindSharedResources(gfx, renderState);

		m_pSkyboxKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

		UnbindSharedResources(gfx, renderState);
		gfx.GetRenderStats().EndTaskTimer(GetName());
	}

	void SkyboxPass::SetupRenderPassDependencies(const GraphicsDevice& gfx, const ComPtr<ID3D11ShaderResourceView>& pDepthStencil, const RenderTexture& pCameraColor)
	{
		CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pDepthStencil);
		CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pCameraColor.GetUAV());
	}
}