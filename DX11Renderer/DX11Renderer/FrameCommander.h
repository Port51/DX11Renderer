#pragma once
#include <array>
#include <string>
#include <vector>
#include "BindableInclude.h"
#include "DepthStencilState.h"
#include "Graphics.h"
#include "RenderJob.h"
#include "RenderPass.h"
#include "FullscreenPass.h"
#include "NullPixelShader.h"
#include "RenderTarget.h"
#include "DepthStencilTarget.h"
#include "ComputeShader.h"
#include "ComputeKernel.h"
#include "StructuredBuffer.h"

class FrameCommander
{
private:
	const static int GbufferSize = 2;
public:
	FrameCommander(Graphics& gfx)
	{
		// Setup render targets
		pGbufferNormalRough = std::make_shared<RenderTarget>(gfx);
		pGbufferNormalRough->Init(gfx.GetDevice().Get(), 1280 / 2, 720 / 2);

		pGbufferSecond = std::make_shared<RenderTarget>(gfx);
		pGbufferSecond->Init(gfx.GetDevice().Get(), 1280 / 2, 720 / 2);

		pSmallDepthStencil = std::make_shared<DepthStencilTarget>(gfx, 1280 / 2, 720 / 2);

		// Setup passes
		pRenderPasses.emplace(DepthPrepassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(FinalBlitRenderPassName, std::make_unique<FullscreenPass>(gfx, pGbufferNormalRough, "Assets\\Built\\Shaders\\BlitPS.cso"));

		// Setup Gbuffer
		pGbufferRenderViews.resize(GbufferSize);
		pGbufferRenderViews[0] = pGbufferNormalRough->pRenderTargetView.Get();
		pGbufferRenderViews[1] = pGbufferSecond->pRenderTargetView.Get();

		testKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\ComputeTest.cso"), std::string("CSMain")));

		testSB = std::make_shared<StructuredBuffer<float>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 64);
		testKernel->SetUAV(0u, testSB);
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
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// Compute test pass
		{
			testKernel->Dispatch(gfx, 64, 1, 1);
		}

		// Early Z pass
		// todo: put position into separate vert buffer and only bind that here
		{
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			pSmallDepthStencil->Clear(gfx);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(1, pGbufferRenderViews.data(), pSmallDepthStencil->GetView().Get());
			gfx.SetViewport(1280 / 2, 720 / 2);

			pRenderPasses[DepthPrepassName]->Execute(gfx);
		}

		// GBuffer pass
		{
			//pCameraColor->SetRenderTarget(gfx.GetContext().Get(), gfx.pDepthStencilView.Get());
			//gfx.SetRenderTarget(pCameraColor->pRenderTargetView);

			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pGbufferNormalRough->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);
			pGbufferSecond->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(2, &pGbufferRenderViews[0], pSmallDepthStencil->GetView().Get());
			gfx.SetViewport(1280 / 2, 720 / 2);

			pRenderPasses[GBufferRenderPassName]->Execute(gfx);
		}

		// Final blit
		{
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			gfx.SetViewport(1280, 720);
			gfx.GetContext()->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), gfx.pDepthStencil->GetView().Get());

			pRenderPasses[FinalBlitRenderPassName]->Execute(gfx);
		}
	}

	void Reset()
	{
		for (auto& p : pRenderPasses)
		{
			p.second->Reset();
		}
	}

private:
	std::vector<ID3D11RenderTargetView*> pGbufferRenderViews;
	std::shared_ptr<DepthStencilTarget> pSmallDepthStencil;
	std::shared_ptr<RenderTarget> pGbufferNormalRough;
	std::shared_ptr<RenderTarget> pGbufferSecond;
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> pRenderPasses;

	std::shared_ptr<StructuredBuffer<float>> testSB;
	std::unique_ptr<ComputeKernel> testKernel;
private:
	const std::string DepthPrepassName = std::string("DepthPrepass");
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};