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
private:
	const static int GbufferSize = 2;
public:
	FrameCommander(Graphics& gfx)
	{
		// Setup render targets
		pGbufferNormalRough = std::make_shared<RenderTarget>(gfx);
		pGbufferNormalRough->Init(gfx.pDevice.Get(), 1280 / 2, 720 / 2);

		pGbufferSecond = std::make_shared<RenderTarget>(gfx);
		pGbufferSecond->Init(gfx.pDevice.Get(), 1280 / 2, 720 / 2);

		pSmallDepthStencil = std::make_shared<DepthStencilTarget>(gfx, 1280 / 2, 720 / 2);
		pFullDepthStencil = std::make_shared<DepthStencilTarget>(gfx, 1280, 720);

		// Setup passes
		renderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>());
		renderPasses.emplace(FinalBlitRenderPassName, std::make_unique<FullscreenPass>(gfx, pGbufferSecond));

		// Setup Gbuffer
		pGbufferRenderViews[0] = pGbufferNormalRough->pRenderTargetView.Get();
		pGbufferRenderViews[1] = pGbufferSecond->pRenderTargetView.Get();
	}

	~FrameCommander()
	{
		for (int i = 0; i < GbufferSize; ++i)
		{
			// Causes an error when exiting built exe
			//delete pGbufferRenderViews[i];
		}
	}

	void Accept(RenderJob job, std::string targetPass)
	{
		renderPasses[targetPass]->EnqueueJob(job);
	}

	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// GBuffer pass
		{
			//pCameraColor->SetRenderTarget(gfx.pContext.Get(), gfx.pDepthStencilView.Get());
			//gfx.SetRenderTarget(pCameraColor->pRenderTargetView);

			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);
			pGbufferNormalRough->ClearRenderTarget(gfx.pContext.Get(), 1.f, 0.f, 0.f, 1.f);
			pGbufferSecond->ClearRenderTarget(gfx.pContext.Get(), 1.f, 0.f, 0.f, 1.f);
			pSmallDepthStencil->Clear(gfx);

			// MRT bind
			gfx.pContext->OMSetRenderTargets(2, &pGbufferRenderViews[0], gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(1280 / 2, 720 / 2);

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
	}

	void Reset()
	{
		for (auto& p : renderPasses)
		{
			p.second->Reset();
		}
	}

private:
	ID3D11RenderTargetView* pGbufferRenderViews[GbufferSize];
	std::shared_ptr<DepthStencilTarget> pSmallDepthStencil;
	std::shared_ptr<DepthStencilTarget> pFullDepthStencil;
	std::shared_ptr<RenderTarget> pGbufferNormalRough;
	std::shared_ptr<RenderTarget> pGbufferSecond;
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> renderPasses;
private:
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};