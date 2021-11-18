#pragma once
#include <array>
#include <string>
#include "BindableInclude.h"
#include "Stencil.h"
#include "Graphics.h"
#include "RenderJob.h"
#include "RenderPass.h"
#include "FullscreenPass.h"
#include "NullPixelShader.h"
#include "RenderTexture.h"

class FrameCommander
{
public:
	FrameCommander(Graphics& gfx)
	{
		pCameraColor = std::make_unique<RenderTexture>(gfx);
		pCameraColor->Init(gfx.pDevice.Get(), 128, 128);

		renderPasses.emplace(GBufferRenderPassName, RenderPass());
		renderPasses.emplace(FinalBlitRenderPassName, FullscreenPass());
	}
	void Accept(RenderJob job, std::string targetPass)
	{
		renderPasses[targetPass].EnqueueJob(job);
	}
	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// Can do global binds here

		// GBuffer pass
		{
			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);
			pCameraColor->SetRenderTarget(gfx.pContext.Get(), gfx.pDepthStencilView.Get());
			renderPasses[GBufferRenderPassName].Execute(gfx);
		}

		// Final blit
		{

		}

		// GBuffer pass
		/*{
			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);
			gfx.SetRenderTarget(gfx.pRenderTargetView);
			renderPasses[GBufferRenderPassName].Execute(gfx);
		}*/
		
		// outline masking pass
		/*Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Write)->Bind(gfx);
		Bind::NullPixelShader::Resolve(gfx)->Bind(gfx);
		passes[1].Execute(gfx);
		// outline drawing pass
		Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Mask)->Bind(gfx);
		passes[2].Execute(gfx);*/
	}
	void Reset()
	{
		for (auto& p : renderPasses)
		{
			p.second.Reset();
		}
	}
private:
	std::unique_ptr<RenderTexture> pCameraColor;
	std::unordered_map<std::string, RenderPass> renderPasses;
private:
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};