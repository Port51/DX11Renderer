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
#include "RenderTarget.h"
#include "DepthStencilTarget.h"

class FrameCommander
{
public:
	FrameCommander(Graphics& gfx)
	{
		pCameraColor = std::make_shared<RenderTarget>(gfx);
		pCameraColor->Init(gfx.pDevice.Get(), 1280 / 4, 720 / 4);

		pSmallDepthStencil = std::make_shared<DepthStencilTarget>(gfx, 1280 / 4, 720 / 4);
		pFullDepthStencil = std::make_shared<DepthStencilTarget>(gfx, 1280, 720);

		renderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>());
		renderPasses.emplace(FinalBlitRenderPassName, std::make_unique<FullscreenPass>(gfx, pCameraColor));
	}
	void Accept(RenderJob job, std::string targetPass)
	{
		renderPasses[targetPass]->EnqueueJob(job);
	}
	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// Can do global binds here

		// GBuffer pass
		{
			//pCameraColor->SetRenderTarget(gfx.pContext.Get(), gfx.pDepthStencilView.Get());
			//gfx.SetRenderTarget(pCameraColor->pRenderTargetView);

			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);
			pCameraColor->ClearRenderTarget(gfx.pContext.Get(), gfx.pDepthStencilView.Get(), 1.f, 0.f, 0.f, 1.f);
			pSmallDepthStencil->Clear(gfx);
			pCameraColor->BindAsTarget(gfx, pSmallDepthStencil->GetView().Get());
			renderPasses[GBufferRenderPassName]->Execute(gfx);
		}

		// Final blit
		{
			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);

			pFullDepthStencil->Clear(gfx);
			gfx.SetViewport(1280, 720);
			gfx.pContext->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), pFullDepthStencil->GetView().Get());

			renderPasses[FinalBlitRenderPassName]->Execute(gfx);
		}

		// BLIT NOTES:
		// - Blit seems to be working for solid color
		// - pCameraColor can clear
		// - Rendering to pCameraColor might not be working

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
			p.second->Reset();
		}
	}
private:
	std::shared_ptr<DepthStencilTarget> pSmallDepthStencil;
	std::shared_ptr<DepthStencilTarget> pFullDepthStencil;
	std::shared_ptr<RenderTarget> pCameraColor;
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> renderPasses;
private:
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};