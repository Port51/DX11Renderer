#include "pch.h"
#include "Renderer.h"
#include <array>
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
#include "ShadowPassContext.h"
#include "Sampler.h"
#include "Binding.h"
#include "Bindable.h"
#include "Config.h"
#include "RasterizerState.h"

namespace gfx
{
	Renderer::Renderer(Graphics & gfx, const std::unique_ptr<LightManager>& pLightManager, std::shared_ptr<RendererList> pRendererList)
		: pRendererList(pRendererList)
	{
		std::shared_ptr<Texture> pDither = std::dynamic_pointer_cast<Texture>(Texture::Resolve(gfx, "Assets\\Textures\\Dither8x8.png"));

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
		pTransformationCB = std::make_unique<ConstantBuffer<GlobalTransformCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pPerCameraCB = std::make_unique<ConstantBuffer<PerCameraCB>>(gfx, D3D11_USAGE_DYNAMIC);

		// Setup passes
		CreateRenderPass(PerCameraPassName)->
			CSSetCB(RenderSlots::CS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.CSSetSRV(RenderSlots::CS_LightDataSRV, pLightManager->GetLightDataSRV())
			.CSSetSRV(RenderSlots::CS_LightShadowDataSRV, pLightManager->GetShadowDataSRV())
			.VSSetCB(RenderSlots::VS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerCameraCB, pPerCameraCB->GetD3DBuffer());

		CreateRenderPass(DepthPrepassName)
			->AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();
		CreateRenderPass(ShadowPassName)
			->AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_FRONT)).SetupRSBinding(); // Reduce shadow acne w/ front face culling during shadow pass
		CreateRenderPass(GBufferRenderPassName)
			->AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();
		CreateRenderPass(GeometryRenderPassName);
		CreateRenderPass(GeometryRenderPassName)->
			PSSetSRV(0u, pSpecularLighting->GetSRV())
			.PSSetSRV(1u, pDiffuseLighting->GetSRV());

		CreateRenderPass(FinalBlitRenderPassName,
			std::move(std::make_unique<FullscreenPass>(gfx, FinalBlitRenderPassName, "Assets\\Built\\Shaders\\BlitPS.cso")));

		// todo: move to static class?
		D3D11_SAMPLER_DESC shadowSamplerDesc = {};
		//if (Config::ShadowType == Config::ShadowType::HardwarePCF)
		{
			shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		}
		//else
		//{
		//	shadowSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		//}
		shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.MipLODBias = 0.f;
		shadowSamplerDesc.MinLOD = 0.f;
		shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		shadowSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		pShadowSampler = std::make_shared<Sampler>(gfx, shadowSamplerDesc);

		CreateRenderPass(TiledLightingPassName)->
			CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pNormalRoughTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV, gfx.pDepthStencil->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, pDither->GetSRV())
			.CSSetUAV(0u, pSpecularLighting->GetUAV())
			.CSSetUAV(1u, pDiffuseLighting->GetUAV())
			.CSSetUAV(2u, pDebugTiledLightingCS->GetUAV())
			.AddBinding(pShadowSampler)
			.SetupCSBinding(0u);

		pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingCompute.cso"), std::string("CSMain")));
	}

	Renderer::~Renderer()
	{

	}

	void Renderer::AcceptDrawCall(DrawCall job, std::string targetPass)
	{
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Renderer::Execute(Graphics & gfx, const std::unique_ptr<Camera>& cam, const std::unique_ptr<LightManager>& pLightManager)
	{
		gfx.GetContext()->ClearState();

		/*
		pLightManager->CullLights(gfx, cam);

		gfx.GetContext()->CSSetShaderResources(0u, 1u, pLightManager->GetD3DSRV().GetAddressOf());
		pTiledLightingKernel->Dispatch(gfx, *pRenderPasses[TiledLightingPassName], gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

		return;*/

		// Shadow pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[ShadowPassName];

			pass->BindSharedResources(gfx);

			ShadowPassContext context(gfx, cam, *this, pass, pTransformationCB, nullptr);

			pLightManager->CullLights(gfx, cam); // changes the SRV, which will be bound in per-frame binds
			pLightManager->RenderShadows(context);

			pass->UnbindSharedResources(gfx);
		}

		//return;

		// Submit draw calls
		{
			// todo: store pass set ahead of time
			DrawContext drawContext(*this);
			drawContext.viewMatrix = cam->GetViewMatrix();
			drawContext.projMatrix = cam->GetProjectionMatrix();
			drawContext.SetRenderPasses(std::move(std::vector<std::string> { DepthPrepassName, GBufferRenderPassName, GeometryRenderPassName }));

			// todo: filter by render passes too
			pVisibleRendererList->Filter(cam->GetFrustumWS(), RendererList::RendererSorting::BackToFront);
			pVisibleRendererList->SubmitDrawCalls(drawContext);
		}

		// Early frame calculations
		{
			PerFrameCB perFrameCB;
			ZeroMemory(&perFrameCB, sizeof(perFrameCB));
			perFrameCB.time = { 0.5f, 0, 0, 0 };
			pPerFrameCB->Update(gfx, perFrameCB);

			GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = cam->GetViewMatrix();
			transformationCB.projMatrix = cam->GetProjectionMatrix();
			transformationCB.viewProjMatrix = transformationCB.projMatrix * transformationCB.viewMatrix;
			transformationCB.invViewMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewMatrix);
			transformationCB.invProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.projMatrix);
			transformationCB.invViewProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewProjMatrix);
			pTransformationCB->Update(gfx, transformationCB);

			PerCameraCB perCameraCB;
			ZeroMemory(&perCameraCB, sizeof(perCameraCB));
			perCameraCB.screenParams = dx::XMVectorSet((float)gfx.GetScreenWidth(), (float)gfx.GetScreenHeight(), 1.0f / gfx.GetScreenWidth(), 1.0f / gfx.GetScreenHeight());
			perCameraCB.zBufferParams = dx::XMVectorSet(1.f - cam->farClipPlane / cam->nearClipPlane, cam->farClipPlane / cam->nearClipPlane, 1.f / cam->farClipPlane - 1.f / cam->nearClipPlane, 1.f / cam->nearClipPlane);
			perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
			perCameraCB.frustumCornerDataVS = cam->GetFrustumCornersVS();
			perCameraCB.cameraPositionWS = cam->GetPositionWS();
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
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			gfx.pDepthStencil->Clear(gfx);

			gfx.GetContext()->OMSetRenderTargets(0, nullptr, gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// Normal-rough pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[GBufferRenderPassName];

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pNormalRoughTarget->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);

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
			pLightManager->BindShadowAtlas(gfx, RenderSlots::CS_FreeSRV + 1u);
			pass->Execute(gfx); // setup binds

			pTiledLightingKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// Geometry pass
		{
			const std::unique_ptr<RenderPass>& pass = pRenderPasses[GeometryRenderPassName];

			// Bind global textures here
			pass->BindSharedResources(gfx);

			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);

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
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			// Debug view overrides: (do this here so it can be changed dynamically later)
			//fsPass->SetInputTarget(pCameraColor);
			fsPass->SetInputTarget(pDebugTiledLightingCS);

			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());
			gfx.GetContext()->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), gfx.pDepthStencil->GetView().Get());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

	}

	void Renderer::Reset()
	{
		for (auto& p : pRenderPasses)
		{
			p.second->Reset();
		}
	}

	const std::unique_ptr<RenderPass>& Renderer::CreateRenderPass(const std::string name)
	{
		pRenderPasses.emplace(name, std::make_unique<RenderPass>(name));
		return pRenderPasses[name];
	}

	const std::unique_ptr<RenderPass>& Renderer::CreateRenderPass(const std::string name, std::unique_ptr<RenderPass> pRenderPass)
	{
		pRenderPasses.emplace(name, std::move(pRenderPass));
		return pRenderPasses[name];
	}
}