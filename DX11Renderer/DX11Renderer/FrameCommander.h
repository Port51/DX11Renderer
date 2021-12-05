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
		pGbufferNormalRough->Init(gfx.GetDevice().Get(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pGbufferSecond = std::make_shared<RenderTarget>(gfx);
		pGbufferSecond->Init(gfx.GetDevice().Get(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSpecularLighting = std::make_shared<RenderTarget>(gfx);
		pSpecularLighting->Init(gfx.GetDevice().Get(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDiffuseLighting = std::make_shared<RenderTarget>(gfx);
		pDiffuseLighting->Init(gfx.GetDevice().Get(), gfx.GetScreenWidth(), gfx.GetScreenHeight());
		
		pCameraColor = std::make_shared<RenderTarget>(gfx);
		pCameraColor->Init(gfx.GetDevice().Get(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSmallDepthStencil = std::make_shared<DepthStencilTarget>(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight());

		// Setup passes
		pRenderPasses.emplace(DepthPrepassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(GeometryRenderPassName, std::make_unique<RenderPass>());
		pRenderPasses[GeometryRenderPassName]->AppendGlobalPSTextureBind(pSpecularLighting->GetShaderResourceView().Get());
		pRenderPasses[GeometryRenderPassName]->AppendGlobalPSTextureBind(pDiffuseLighting->GetShaderResourceView().Get());

		pRenderPasses.emplace(FinalBlitRenderPassName, std::make_unique<FullscreenPass>(gfx, pCameraColor, "Assets\\Built\\Shaders\\BlitPS.cso"));

		// Setup Gbuffer
		pGbufferRenderViews.resize(GbufferSize);
		pGbufferRenderViews[0] = pGbufferNormalRough->pRenderTargetView.Get();
		pGbufferRenderViews[1] = pGbufferSecond->pRenderTargetView.Get();

		pTestKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\ComputeTest.cso"), std::string("CSMain")));

		pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingPass.cso"), std::string("CSMain")));

		//testSRV = std::make_shared<StructuredBuffer<float>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 64);
		pTestUAV = std::make_shared<StructuredBuffer<float>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 64);
		//testKernel->SetSRV(0u, pGbufferNormalRough->GetShaderResourceView());
		pTestKernel->SetUAV(0u, pTestUAV);
		pTestKernel->SetUAV(1u, pGbufferNormalRough->GetUAV());
	}

	~FrameCommander()
	{
		
	}

	void AcceptRenderJob(RenderJob job, std::string targetPass)
	{
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// Early Z pass
		// todo: put position into separate vert buffer and only bind that here
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[DepthPrepassName];

			pass->BindGlobals(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx, *pass);
			//pSmallDepthStencil->Clear(gfx);
			gfx.pDepthStencil->Clear(gfx);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(1, pGbufferRenderViews.data(), gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
		}

		// GBuffer pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[GBufferRenderPassName];

			//pCameraColor->SetRenderTarget(gfx.GetContext().Get(), gfx.pDepthStencilView.Get());
			//gfx.SetRenderTarget(pCameraColor->pRenderTargetView);

			pass->BindGlobals(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx, *pass);
			pGbufferNormalRough->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);
			pGbufferSecond->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(2, &pGbufferRenderViews[0], gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
		}

		// Tiled lighting pass
		{
			gfx.GetContext()->OMSetRenderTargets(0u, nullptr, nullptr); // required for binding rendertarget to compute shader
			pTiledLightingKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			// todo: find better way than this
			// clearing UAV bindings doesn't seem to work
			gfx.GetContext()->ClearState();
		}

		// Geometry pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[GeometryRenderPassName];

			// Bind global textures here
			pass->BindGlobals(gfx);

			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx, *pass);

			pCameraColor->BindAsTarget(gfx, gfx.pDepthStencil->GetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
		}

		// Compute test pass
		{
			gfx.GetContext()->OMSetRenderTargets(0u, nullptr, nullptr); // required for binding rendertarget to compute shader
			pTestKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			// todo: find better way than this
			// clearing UAV bindings doesn't seem to work
			gfx.GetContext()->ClearState();
		}

		// Final blit
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[FinalBlitRenderPassName];

			pass->BindGlobals(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx, *pass);

			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());
			gfx.GetContext()->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), gfx.pDepthStencil->GetView().Get());

			pass->Execute(gfx);
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

	std::shared_ptr<RenderTarget> pSpecularLighting;
	std::shared_ptr<RenderTarget> pDiffuseLighting;

	std::shared_ptr<RenderTarget> pCameraColor;

	std::shared_ptr<StructuredBuffer<float>> pTestSRV;
	std::shared_ptr<StructuredBuffer<float>> pTestUAV;
	std::unique_ptr<ComputeKernel> pTestKernel;
	std::unique_ptr<ComputeKernel> pTiledLightingKernel;
private:
	const std::string DepthPrepassName = std::string("DepthPrepass");
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string GeometryRenderPassName = std::string("Geometry");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};