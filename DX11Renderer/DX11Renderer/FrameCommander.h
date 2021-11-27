#pragma once
#include <array>
#include <string>
#include "BindableInclude.h"
#include "DepthStencilState.h"
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

		// Setup passes
		renderPasses.emplace(DepthPrepassName, std::make_unique<RenderPass>());
		renderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>());
		renderPasses.emplace(FinalBlitRenderPassName, std::make_unique<FullscreenPass>(gfx, pGbufferNormalRough, "Assets\\Built\\Shaders\\BlitPS.cso"));

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

	void AcceptRenderJob(RenderJob job, std::string targetPass)
	{
		renderPasses[targetPass]->EnqueueJob(job);
	}

	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// Early Z pass
		// todo: put position into separate vert buffer and only bind that here
		{
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			pSmallDepthStencil->Clear(gfx);

			// MRT bind
			gfx.pContext->OMSetRenderTargets(1, &pGbufferRenderViews[0], pSmallDepthStencil->GetView().Get());
			gfx.SetViewport(1280 / 2, 720 / 2);

			renderPasses[DepthPrepassName]->Execute(gfx);
		}

		// GBuffer pass
		{
			//pCameraColor->SetRenderTarget(gfx.pContext.Get(), gfx.pDepthStencilView.Get());
			//gfx.SetRenderTarget(pCameraColor->pRenderTargetView);

			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pGbufferNormalRough->ClearRenderTarget(gfx.pContext.Get(), 1.f, 0.f, 0.f, 1.f);
			pGbufferSecond->ClearRenderTarget(gfx.pContext.Get(), 1.f, 0.f, 0.f, 1.f);

			// MRT bind
			gfx.pContext->OMSetRenderTargets(2, &pGbufferRenderViews[0], pSmallDepthStencil->GetView().Get());
			gfx.SetViewport(1280 / 2, 720 / 2);

			renderPasses[GBufferRenderPassName]->Execute(gfx);
		}

		// Final blit
		{
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			gfx.SetViewport(1280, 720);
			gfx.pContext->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), gfx.pDepthStencil->GetView().Get());

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
	std::shared_ptr<RenderTarget> pGbufferNormalRough;
	std::shared_ptr<RenderTarget> pGbufferSecond;
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> renderPasses;
private:
	const std::string DepthPrepassName = std::string("DepthPrepass");
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};