#pragma once
#include <array>
#include <string>
#include <vector>
#include "BindableInclude.h"
#include "DepthStencilState.h"
#include "Graphics.h"
#include "DrawCall.h"
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
#include "RendererList.h"
#include "Frustum.h"
#include "DrawContext.h"

class Renderer
{
public:
	Renderer(Graphics& gfx, const std::unique_ptr<LightManager>& pLightManager, std::shared_ptr<RendererList> pRendererList)
		: pRendererList(pRendererList)
	{

		pVisibleRendererList = std::make_unique<RendererList>(pRendererList);

		// Setup render targets
		pNormalRoughTarget = std::make_shared<RenderTarget>(gfx);
		pNormalRoughTarget->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSpecularLighting = std::make_shared<RenderTarget>(gfx);
		pSpecularLighting->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDiffuseLighting = std::make_shared<RenderTarget>(gfx);
		pDiffuseLighting->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());
		
		pCameraColor = std::make_shared<RenderTarget>(gfx);
		pCameraColor->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDebugTiledLightingCS = std::make_shared<RenderTarget>(gfx);
		pDebugTiledLightingCS->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSmallDepthStencil = std::make_shared<DepthStencilTarget>(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pPerFrameCB = std::make_unique<ConstantBuffer<PerFrameCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pTransformationCB = std::make_unique<ConstantBuffer<TransformationCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pPerCameraCB = std::make_unique<ConstantBuffer<PerCameraCB>>(gfx, D3D11_USAGE_DYNAMIC);

		// Setup passes
		pRenderPasses.emplace(PerCameraPassName, std::make_unique<RenderPass>());
		const std::unique_ptr<RenderPass>& setupPass = pRenderPasses[PerCameraPassName];
		setupPass->
			CSSetCB(RenderSlots::CS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.CSSetSRV(RenderSlots::CS_LightDataSRV, pLightManager->GetD3DSRV())
			//.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pGbufferNormalRough->GetSRV())
			.VSSetCB(RenderSlots::VS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerCameraCB, pPerCameraCB->GetD3DBuffer());

		pRenderPasses.emplace(DepthPrepassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(ShadowPassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(GBufferRenderPassName, std::make_unique<RenderPass>());
		pRenderPasses.emplace(GeometryRenderPassName, std::make_unique<RenderPass>());
		pRenderPasses[GeometryRenderPassName]->
			PSSetSRV(0u, pSpecularLighting->GetSRV())
			.PSSetSRV(1u, pDiffuseLighting->GetSRV());

		pRenderPasses.emplace(FinalBlitRenderPassName,
			std::make_unique<FullscreenPass>(gfx, "Assets\\Built\\Shaders\\BlitPS.cso"));

		//pTestKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\ComputeTest.cso"), std::string("CSMain")));

		pRenderPasses.emplace(TiledLightingPassName, std::make_unique<RenderPass>());
		pRenderPasses[TiledLightingPassName]->
			CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pNormalRoughTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV, gfx.pDepthStencil->GetSRV())
			.CSSetUAV(0u, pSpecularLighting->GetUAV())
			.CSSetUAV(1u, pDiffuseLighting->GetUAV())
			.CSSetUAV(2u, pDebugTiledLightingCS->GetUAV());

		pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingCompute.cso"), std::string("CSMain")));
	}

	~Renderer()
	{
		
	}

	void AcceptDrawCall(DrawCall job, std::string targetPass)
	{
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Execute(Graphics& gfx, const Camera& cam, const std::unique_ptr<LightManager>& pLightManager)
	{
		gfx.GetContext()->ClearState();

		// todo: replace w/ rendergraph
		/*
		pLightManager->CullLights(gfx, cam);

		gfx.GetContext()->CSSetShaderResources(0u, 1u, pLightManager->GetD3DSRV().GetAddressOf());
		pTiledLightingKernel->Dispatch(gfx, *pRenderPasses[TiledLightingPassName], gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

		return;*/

		// Shadow pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[ShadowPassName];

			pass->BindSharedResources(gfx);

			pLightManager->CullLights(gfx, cam); // changes the SRV, which will be bound in per-frame binds
			pLightManager->RenderShadows(gfx, cam, pass, pTransformationCB);

			pass->UnbindSharedResources(gfx);
		}

		// Frustum culling
		{
			pVisibleRendererList->Filter(cam.GetFrustumWS(), RendererList::RendererSorting::BackToFront);
		}

		// Submit draw calls
		{
			DrawContext drawContext(*this);
			drawContext.viewMatrix = cam.GetViewMatrix();
			drawContext.projMatrix = cam.GetProjectionMatrix();
			pVisibleRendererList->SubmitDrawCalls(drawContext);
		}

		// Early frame calculations
		{
			PerFrameCB perFrameCB;
			ZeroMemory(&perFrameCB, sizeof(perFrameCB));
			perFrameCB.time = { 0.5f, 0, 0, 0 };
			pPerFrameCB->Update(gfx, perFrameCB);

			TransformationCB transformationCB;
			transformationCB.viewMatrix = cam.GetViewMatrix();
			transformationCB.projMatrix = cam.GetProjectionMatrix();
			pTransformationCB->Update(gfx, transformationCB);

			PerCameraCB perCameraCB;
			ZeroMemory(&perCameraCB, sizeof(perCameraCB));
			perCameraCB.screenParams = dx::XMVectorSet((float)gfx.GetScreenWidth(), (float)gfx.GetScreenHeight(), 1.0f / gfx.GetScreenWidth(), 1.0f / gfx.GetScreenHeight());
			perCameraCB.zBufferParams = dx::XMVectorSet(1.f - cam.farClipPlane / cam.nearClipPlane, cam.farClipPlane / cam.nearClipPlane, 1.f / cam.farClipPlane - 1.f / cam.nearClipPlane, 1.f / cam.nearClipPlane);
			perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
			perCameraCB.frustumCornerDataVS = cam.GetFrustumCornersVS();
			pPerCameraCB->Update(gfx, perCameraCB);
		}

		// Per-frame and per-camera binds
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[PerCameraPassName];
			
			pass->BindSharedResources(gfx);

			pass->Execute(gfx);
		}

		// Early Z pass
		// todo: put position into separate vert buffer and only bind that here
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[DepthPrepassName];

			pass->BindSharedResources(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			gfx.pDepthStencil->Clear(gfx);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(0, nullptr, gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// GBuffer pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[GBufferRenderPassName];

			//pCameraColor->SetRenderTarget(gfx.GetContext().Get(), gfx.pDepthStencilView.Get());
			//gfx.SetRenderTarget(pCameraColor->pRenderTargetView);

			pass->BindSharedResources(gfx);
			Bind::DepthStencilState::Resolve(gfx, Bind::DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pNormalRoughTarget->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);

			// MRT bind
			gfx.GetContext()->OMSetRenderTargets(1, pNormalRoughTarget->GetView().GetAddressOf(), gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);

			// todo: unbind these in a cleaner way
			std::vector<ID3D11RenderTargetView*> nullRTVs(1u, nullptr);
			gfx.GetContext()->OMSetRenderTargets(1u, nullRTVs.data(), nullptr);

			pass->UnbindSharedResources(gfx);
		}

		// Tiled lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[TiledLightingPassName];

			pass->BindSharedResources(gfx);

			pTiledLightingKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
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
			pass->UnbindSharedResources(gfx);
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
			fsPass->SetInputTarget(pCameraColor);
			//fsPass->SetInputTarget(pDebugTiledLightingCS);

			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());
			gfx.GetContext()->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), gfx.pDepthStencil->GetView().Get());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
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
	std::shared_ptr<DepthStencilTarget> pSmallDepthStencil;
	std::shared_ptr<RenderTarget> pNormalRoughTarget;
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> pRenderPasses;

	std::shared_ptr<RenderTarget> pSpecularLighting;
	std::shared_ptr<RenderTarget> pDiffuseLighting;

	// Debug views
	std::shared_ptr<RenderTarget> pDebugTiledLightingCS;

	std::shared_ptr<RenderTarget> pCameraColor;

	std::unique_ptr<ComputeKernel> pTiledLightingKernel;
	std::unique_ptr<ConstantBuffer<TransformationCB>> pTransformationCB;
	std::unique_ptr<ConstantBuffer<PerFrameCB>> pPerFrameCB;
	std::unique_ptr<ConstantBuffer<PerCameraCB>> pPerCameraCB;

	std::shared_ptr<RendererList> pRendererList;
	std::unique_ptr<RendererList> pVisibleRendererList; // filtered by camera frustum
private:
	const std::string PerCameraPassName = std::string("PerCameraPass");
	const std::string ShadowPassName = std::string("ShadowPass");
	const std::string DepthPrepassName = std::string("DepthPrepass");
	const std::string GBufferRenderPassName = std::string("GBuffer");
	const std::string TiledLightingPassName = std::string("TiledLighting");
	const std::string GeometryRenderPassName = std::string("Geometry");
	const std::string FinalBlitRenderPassName = std::string("FinalBlit");
};