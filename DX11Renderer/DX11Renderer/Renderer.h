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
#include "CommonCbuffers.h"
#include "LightData.h"
#include "PointLight.h"
#include "Camera.h"
#include "LightManager.h"
#include "RenderConstants.h"

class Renderer
{
private:
	const static int GbufferSize = 2;
public:
	Renderer(Graphics& gfx, const std::unique_ptr<LightManager>& pLightManager)
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

		pDebugTiledLightingCS = std::make_shared<RenderTarget>(gfx);
		pDebugTiledLightingCS->Init(gfx.GetDevice().Get(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSmallDepthStencil = std::make_shared<DepthStencilTarget>(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pPerFrameCB = std::make_unique<ConstantBuffer<PerFrameCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pPerCameraCB = std::make_unique<ConstantBuffer<PerCameraCB>>(gfx, D3D11_USAGE_DYNAMIC);

		// Setup passes
		pRenderPasses.emplace(PerCameraPassName, std::make_unique<RenderPass>());
		const std::unique_ptr<RenderPass>& setupPass = pRenderPasses[PerCameraPassName];
		setupPass->
			CSSetCB(RenderSlots::CS_PerFrameCB, pPerFrameCB->GetD3DBuffer().Get())
			.CSSetCB(RenderSlots::CS_PerCameraCB, pPerCameraCB->GetD3DBuffer().Get())
			.CSSetSRV(RenderSlots::CS_LightDataSRV, pLightManager->GetD3DSRV().Get())
			.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pGbufferNormalRough->GetSRV().Get())
			.VSSetCB(RenderSlots::VS_PerFrameCB, pPerFrameCB->GetD3DBuffer().Get())
			.VSSetCB(RenderSlots::VS_PerCameraCB, pPerCameraCB->GetD3DBuffer().Get())
			.PSSetCB(RenderSlots::PS_PerFrameCB, pPerFrameCB->GetD3DBuffer().Get())
			.PSSetCB(RenderSlots::PS_PerCameraCB, pPerCameraCB->GetD3DBuffer().Get());

		pRenderPasses.emplace(DepthPrepassName, std::make_unique<RenderPass>(*setupPass));
		pRenderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>(*setupPass));
		pRenderPasses.emplace(GeometryRenderPassName, std::make_unique<RenderPass>(*setupPass));
		pRenderPasses[GeometryRenderPassName]->
			PSSetSRV(0u, pSpecularLighting->GetSRV().Get())
			.PSSetSRV(1u, pDiffuseLighting->GetSRV().Get());

		pRenderPasses.emplace(FinalBlitRenderPassName,
			std::make_unique<FullscreenPass>(gfx, "Assets\\Built\\Shaders\\BlitPS.cso"));

		// Setup Gbuffer
		pGbufferRenderViews.resize(GbufferSize);
		pGbufferRenderViews[0] = pGbufferNormalRough->pRenderTargetView.Get();
		pGbufferRenderViews[1] = pGbufferSecond->pRenderTargetView.Get();

		//pTestKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\ComputeTest.cso"), std::string("CSMain")));

		pRenderPasses.emplace(TiledLightingPassName, std::make_unique<RenderPass>(*setupPass));
		pRenderPasses[TiledLightingPassName]->
			CSSetSRV(RenderSlots::CS_FreeSRV, gfx.pDepthStencil->GetSRV())
			.CSSetUAV(0u, pSpecularLighting->GetUAV().Get())
			.CSSetUAV(1u, pDiffuseLighting->GetUAV().Get())
			.CSSetUAV(2u, pDebugTiledLightingCS->GetUAV().Get());

		pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingCompute.cso"), std::string("CSMain")));
	}

	~Renderer()
	{
		
	}

	void AcceptRenderJob(RenderJob job, std::string targetPass)
	{
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Execute(Graphics& gfx, const Camera& cam, const std::unique_ptr<LightManager>& pLightManager)
	{
		// todo: replace w/ rendergraph
		/*gfx.GetContext()->ClearState();
		pLightManager->CullLights(gfx, cam);

		gfx.GetContext()->CSSetShaderResources(0u, 1u, pLightManager->GetD3DSRV().GetAddressOf());
		pTiledLightingKernel->Dispatch(gfx, *pRenderPasses[TiledLightingPassName], gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

		return;*/

		PerFrameCB perFrameCB;
		ZeroMemory(&perFrameCB, sizeof(perFrameCB));
		perFrameCB.time = { 0.5f, 0, 0, 0 };
		pPerFrameCB->Update(gfx, perFrameCB);

		PerCameraCB perCameraCB;
		ZeroMemory(&perCameraCB, sizeof(perCameraCB));
		perCameraCB.screenParams = dx::XMVectorSet( (float)gfx.GetScreenWidth(), (float)gfx.GetScreenHeight(), 1.0f / gfx.GetScreenWidth(), 1.0f / gfx.GetScreenHeight() );
		perCameraCB.zBufferParams = dx::XMVectorSet( 1.f - cam.farClipPlane / cam.nearClipPlane, cam.farClipPlane / cam.nearClipPlane, 1.f / cam.farClipPlane - 1.f / cam.nearClipPlane, 1.f / cam.nearClipPlane);
		perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
		perCameraCB.frustumCornerDataVS = cam.GetFrustumCornersVS();
		perCameraCB.viewProj = cam.GetViewMatrix();
		perCameraCB.projection = cam.GetProjectionMatrix();
		pPerCameraCB->Update(gfx, perCameraCB);

		// Per-frame and per-camera binds
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[PerCameraPassName];

			pLightManager->CullLights(gfx, cam); // changes the SRV, which will be bound below
			pass->BindSharedResources(gfx);

			pass->Execute(gfx);
		}

		// Early Z pass
		// todo: put position into separate vert buffer and only bind that here
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[DepthPrepassName];

			pass->BindSharedResources(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);
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

			pass->BindSharedResources(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pGbufferNormalRough->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);
			pGbufferSecond->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(2, &pGbufferRenderViews[0], gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
		}

		// Tiled lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[TiledLightingPassName];

			gfx.GetContext()->OMSetRenderTargets(0u, nullptr, nullptr); // required for binding rendertarget to compute shader
			pass->BindSharedResources(gfx);
			
			// debug:
			//gfx.GetContext()->ClearState();

			pTiledLightingKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			// todo: find better way than this
			// clearing UAV bindings doesn't seem to work
			//gfx.GetContext()->ClearState();

			pass->Execute(gfx);
		}

		// Geometry pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[GeometryRenderPassName];

			// Bind global textures here
			pass->BindSharedResources(gfx);

			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx);

			pCameraColor->ClearRenderTarget(gfx.GetContext().Get(), 0.f, 0.f, 0.f, 0.f);
			pCameraColor->BindAsTarget(gfx, gfx.pDepthStencil->GetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
		}

		// Final blit
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[FinalBlitRenderPassName];
			const auto fsPass = static_cast<FullscreenPass*>(pass.get());

			// todo: remove this - for now it's needed to clear SRVs
			gfx.GetContext()->ClearState();

			pass->BindSharedResources(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			// Debug view overrides: (do this here so it can be changed dynamically later)
			//fsPass->SetInputTarget(pCameraColor);
			fsPass->SetInputTarget(pDebugTiledLightingCS);

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

	// Debug views
	std::shared_ptr<RenderTarget> pDebugTiledLightingCS;

	std::shared_ptr<RenderTarget> pCameraColor;

	//std::shared_ptr<StructuredBuffer<float>> pTestSRV;
	//std::shared_ptr<StructuredBuffer<float>> pTestUAV;
	//std::unique_ptr<ComputeKernel> pTestKernel;
	std::unique_ptr<ComputeKernel> pTiledLightingKernel;
	std::unique_ptr<ConstantBuffer<PerFrameCB>> pPerFrameCB;
	std::unique_ptr<ConstantBuffer<PerCameraCB>> pPerCameraCB;
private:
	const std::string PerCameraPassName = std::string("PerCameraPass");
	const std::string DepthPrepassName = std::string("DepthPrepass");
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string TiledLightingPassName = std::string("TiledLighting");
	const std::string GeometryRenderPassName = std::string("Geometry");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};