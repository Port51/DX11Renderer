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
#include "RenderTexture.h"
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
#include "Sampler.h"

namespace gfx
{
	Renderer::Renderer(Graphics& gfx, std::shared_ptr<LightManager> pLightManager, std::shared_ptr<RendererList> pRendererList)
		: pRendererList(pRendererList), pLightManager(pLightManager)
	{
		// todo: move some of these to an asset manager class

		//
		// Debug stuff
		//
		rendererFeatureEnabled.resize(RendererFeature::COUNT, true); // enable all features by default

		//
		// Components
		//
		pVisibleRendererList = std::make_unique<RendererList>(pRendererList);

		//
		// Texture assets and samplers
		//
		pDither = std::dynamic_pointer_cast<Texture>(Texture::Resolve(gfx, "Assets\\Textures\\Dither8x8.png"));
		pSampler_ClampedBilinear = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);
		
		D3D11_SAMPLER_DESC shadowSamplerDesc = {};
		shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.MipLODBias = 0.f;
		shadowSamplerDesc.MinLOD = 0.f;
		shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		shadowSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		pShadowSampler = std::make_shared<Sampler>(gfx, shadowSamplerDesc);

		//
		// Render targets
		//
		pNormalRoughReflectivityTarget = std::make_shared<RenderTexture>(gfx);
		pNormalRoughReflectivityTarget->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pHiZBufferTarget = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16_UNORM, 8u);
		pHiZBufferTarget->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSpecularLighting = std::make_shared<RenderTexture>(gfx);
		pSpecularLighting->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDiffuseLighting = std::make_shared<RenderTexture>(gfx);
		pDiffuseLighting->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pCameraColor0 = std::make_shared<RenderTexture>(gfx);
		pCameraColor0->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pCameraColor1 = std::make_shared<RenderTexture>(gfx);
		pCameraColor1->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDebugTiledLightingCS = std::make_shared<RenderTexture>(gfx);
		pDebugTiledLightingCS->Init(gfx.GetDevice(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		//
		// Buffers
		//
		pPerFrameCB = std::make_unique<ConstantBuffer<PerFrameCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pTransformationCB = std::make_unique<ConstantBuffer<GlobalTransformCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pPerCameraCB = std::make_unique<ConstantBuffer<PerCameraCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pHiZCreationCB = std::make_unique<ConstantBuffer<HiZCreationCB>>(gfx, D3D11_USAGE_DYNAMIC);

		pFXAA_CB = std::make_unique<ConstantBuffer<FXAA_CB>>(gfx, D3D11_USAGE_DYNAMIC);
		pSSR_CB = std::make_unique<ConstantBuffer<SSR_CB>>(gfx, D3D11_USAGE_DYNAMIC);
		pSSR_DebugData = std::make_unique<StructuredBuffer<int>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 4u);
		pDitherCB = std::make_unique<ConstantBuffer<DitherCB>>(gfx, D3D11_USAGE_DYNAMIC);

		//
		// Compute shaders
		//
		pHiZDepthCopyKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\HiZDepthCopy.cso"), std::string("CSMain")));
		pHiZCreateMipKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\HiZCreateMip.cso"), std::string("CSMain")));
		pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingCompute.cso"), std::string("CSMain")));
		pSSRKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\SSR.cso"), std::string("CSMain")));
		pFXAAKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\FXAA.cso"), std::string("CSMain")));
		pDitherKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\Dither.cso"), std::string("CSMain")));
		pTonemappingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\Tonemapping.cso"), std::string("CSMain")));

		//
		// Render passes
		//
		CreateRenderPass(PerCameraPassName);
		CreateRenderPass(DepthPrepassName);
		CreateRenderPass(HiZPassName);
		CreateRenderPass(ShadowPassName);
		CreateRenderPass(GBufferRenderPassName);
		CreateRenderPass(TiledLightingPassName);
		CreateRenderPass(GeometryRenderPassName);
		CreateRenderPass(SSRRenderPassName);
		CreateRenderPass(FXAARenderPassName);
		CreateRenderPass(DitherRenderPassName);
		CreateRenderPass(TonemappingRenderPassName);

		CreateRenderPass(FinalBlitRenderPassName,
			std::move(std::make_unique<FullscreenPass>(gfx, FinalBlitRenderPassName, "Assets\\Built\\Shaders\\BlitPS.cso")));

		SetupRenderPassDependencies(gfx);
	}

	Renderer::~Renderer()
	{

	}

	void Renderer::SetupRenderPassDependencies(Graphics& gfx)
	{
		bool cameraOutSlot0 = true;

		GetRenderPass(PerCameraPassName)->
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

		GetRenderPass(DepthPrepassName)
			->AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(HiZPassName)
			->CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.pDepthStencil->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pHiZBufferTarget->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, nullptr)
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, pHiZCreationCB->GetD3DBuffer());

		GetRenderPass(ShadowPassName)
			->AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_FRONT)).SetupRSBinding(); // Reduce shadow acne w/ front face culling during shadow pass

		GetRenderPass(GBufferRenderPassName)
			->AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(TiledLightingPassName)->
			CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pNormalRoughReflectivityTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV, gfx.pDepthStencil->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, pHiZBufferTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, nullptr) // reserve this slot for shadow atlas!
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, pDither->GetSRV())
			.CSSetUAV(0u, pSpecularLighting->GetUAV())
			.CSSetUAV(1u, pDiffuseLighting->GetUAV())
			.CSSetUAV(2u, pDebugTiledLightingCS->GetUAV())
			.AddBinding(pShadowSampler).SetupCSBinding(0u);

		GetRenderPass(GeometryRenderPassName)->
			PSSetSRV(0u, pSpecularLighting->GetSRV())
			.PSSetSRV(1u, pDiffuseLighting->GetSRV());

		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			// Assign inputs and outputs
			auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(SSRRenderPassName)->
				CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pNormalRoughReflectivityTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, gfx.pDepthStencil->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, pHiZBufferTarget->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, pSSR_DebugData->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, pSSR_CB->GetD3DBuffer())
				.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, pSampler_ClampedBilinear->GetD3DSampler());
		}
		
		if (IsFeatureEnabled(RendererFeature::FXAA))
		{
			// Assign inputs and outputs
			auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(FXAARenderPassName)->
				CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, pFXAA_CB->GetD3DBuffer())
				.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, pSampler_ClampedBilinear->GetD3DSampler());
		}

		if (IsFeatureEnabled(RendererFeature::Dither))
		{
			// Assign inputs and outputs
			auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(DitherRenderPassName)->
				CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, pDither->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, pDitherCB->GetD3DBuffer());
		}

		if (IsFeatureEnabled(RendererFeature::Dither))
		{
			// Assign inputs and outputs
			auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(TonemappingRenderPassName)->
				CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV());
			//.CSSetCB(RenderSlots::CS_FreeCB + 0u, pDitherCB->GetD3DBuffer());
		}

		// Determine what to use for final blit
		pFinalBlitInputIs0 = cameraOutSlot0;
	}

	void Renderer::AcceptDrawCall(DrawCall job, std::string targetPass)
	{
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Renderer::Execute(Graphics & gfx, const std::unique_ptr<Camera>& cam, float timeElapsed)
	{
		gfx.GetContext()->ClearState();
		static int frameCt = 0;
		frameCt++;

		// Shadow + lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(ShadowPassName);

			pass->BindSharedResources(gfx);

			ShadowPassContext context(gfx, cam, *this, pass, pTransformationCB, nullptr);

			pLightManager->CullLights(gfx, cam, IsFeatureEnabled(RendererFeature::Shadows)); // changes the SRV, which will be bound in per-frame binds
			if (IsFeatureEnabled(RendererFeature::Shadows))
			{
				pLightManager->RenderShadows(context);
			}

			pass->UnbindSharedResources(gfx);
		}

		// Submit draw calls
		{
			// todo: store pass set ahead of time
			static DrawContext drawContext(*this);
			drawContext.viewMatrix = cam->GetViewMatrix();
			drawContext.projMatrix = cam->GetProjectionMatrix();
			drawContext.SetRenderPasses(std::move(std::vector<std::string> { DepthPrepassName, GBufferRenderPassName, GeometryRenderPassName }));

			// todo: filter by render passes too
			pVisibleRendererList->Filter(cam->GetFrustumWS(), RendererList::RendererSorting::BackToFront);
			pVisibleRendererList->SubmitDrawCalls(drawContext);
		}

		// Early frame calculations
		{
			static PerFrameCB perFrameCB;
			ZeroMemory(&perFrameCB, sizeof(perFrameCB));
			perFrameCB.time = { timeElapsed, 0, 0, 0 };
			pPerFrameCB->Update(gfx, perFrameCB);

			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = cam->GetViewMatrix();
			transformationCB.projMatrix = cam->GetProjectionMatrix();
			transformationCB.viewProjMatrix = transformationCB.projMatrix * transformationCB.viewMatrix;
			transformationCB.invViewMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewMatrix);
			transformationCB.invProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.projMatrix);
			transformationCB.invViewProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewProjMatrix);
			pTransformationCB->Update(gfx, transformationCB);

			static PerCameraCB perCameraCB;
			ZeroMemory(&perCameraCB, sizeof(perCameraCB));
			perCameraCB.projectionParams = dx::XMVectorSet(1.f, cam->GetNearClipPlane(), cam->GetFarClipPlane(), 1.f / cam->farClipPlane);
			perCameraCB.screenParams = dx::XMVectorSet((float)gfx.GetScreenWidth(), (float)gfx.GetScreenHeight(), 1.0f / gfx.GetScreenWidth(), 1.0f / gfx.GetScreenHeight());
			perCameraCB.zBufferParams = dx::XMVectorSet(1.f - cam->farClipPlane / cam->nearClipPlane, cam->farClipPlane / cam->nearClipPlane, 1.f / cam->farClipPlane - 1.f / cam->nearClipPlane, 1.f / cam->nearClipPlane);
			perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
			perCameraCB.frustumCornerDataVS = cam->GetFrustumCornersVS();
			perCameraCB.cameraPositionWS = cam->GetPositionWS();
			pPerCameraCB->Update(gfx, perCameraCB);
		}

		// Per-frame and per-camera binds
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(PerCameraPassName);

			pass->BindSharedResources(gfx);

			pass->Execute(gfx);
		}

		// Early Z pass
		// todo: put position into separate vert buffer and only bind that here
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(DepthPrepassName);

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			gfx.pDepthStencil->Clear(gfx);

			gfx.GetContext()->OMSetRenderTargets(0, nullptr, gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// Hi-Z buffer pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(HiZPassName);

			//gfx.GetContext()->ClearState();
			gfx.GetContext()->OMSetRenderTargets(0, nullptr, nullptr); // need in order to access depth

			pass->BindSharedResources(gfx);
			pass->Execute(gfx);

			UINT screenWidth = (UINT)gfx.GetScreenWidth();
			UINT screenHeight = (UINT)gfx.GetScreenHeight();

			static HiZCreationCB hiZCreationCB;
			hiZCreationCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			//hiZCreationCB.zBufferParams = dx::XMVectorSet(1.f - cam->farClipPlane / cam->nearClipPlane, cam->farClipPlane / cam->nearClipPlane, 1.f / cam->farClipPlane - 1.f / cam->nearClipPlane, 1.f / cam->nearClipPlane);
			pHiZCreationCB->Update(gfx, hiZCreationCB);

			// Copy from depth-stencil
			gfx.GetContext()->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV, 1u, pHiZBufferTarget->GetUAV(0u).GetAddressOf(), nullptr);
			pHiZDepthCopyKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			// Create other mips
			for (UINT mip = 1u; mip < 8u; ++mip)
			{
				UINT dstWidth = screenWidth >> mip;
				UINT dstHeight = screenHeight >> mip;

				if (dstWidth == 0u || dstHeight == 0u)
					continue;

				hiZCreationCB.resolutionSrcDst = { dstWidth << 1u, dstHeight << 1u, dstWidth, dstHeight };
				pHiZCreationCB->Update(gfx, hiZCreationCB);

				// Bind mip slice views as UAVs
				gfx.GetContext()->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, pass->pNullUAVs.data(), nullptr);
				gfx.GetContext()->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, pHiZBufferTarget->GetUAV(mip - 1u).GetAddressOf(), nullptr);
				gfx.GetContext()->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, pHiZBufferTarget->GetUAV(mip).GetAddressOf(), nullptr);

				pHiZCreateMipKernel->Dispatch(gfx, *pass, dstWidth, dstHeight, 1u);
			}

			pass->UnbindSharedResources(gfx);
		}

		// Normal-rough-reflectivity pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(GBufferRenderPassName);

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pNormalRoughReflectivityTarget->ClearRenderTarget(gfx.GetContext().Get(), 1.f, 0.f, 0.f, 1.f);

			gfx.GetContext()->OMSetRenderTargets(1, pNormalRoughReflectivityTarget->GetView().GetAddressOf(), gfx.pDepthStencil->GetView().Get());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);

			// todo: unbind these in a cleaner way
			std::vector<ID3D11RenderTargetView*> nullRTVs(1u, nullptr);
			gfx.GetContext()->OMSetRenderTargets(1u, nullRTVs.data(), nullptr);

			pass->UnbindSharedResources(gfx);
		}

		// Tiled lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(TiledLightingPassName);

			pass->BindSharedResources(gfx);
			pLightManager->BindShadowAtlas(gfx, RenderSlots::CS_FreeSRV + 2u);
			pass->Execute(gfx); // setup binds

			pTiledLightingKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// Geometry pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(GeometryRenderPassName);

			// Bind global textures here
			pass->BindSharedResources(gfx);

			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);

			pCameraColor0->ClearRenderTarget(gfx.GetContext().Get(), 0.f, 0.f, 0.f, 0.f);
			pCameraColor0->BindAsTarget(gfx, gfx.pDepthStencil->GetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);

			// todo: unbind these in a cleaner way
			std::vector<ID3D11RenderTargetView*> nullRTVs(1u, nullptr);
			gfx.GetContext()->OMSetRenderTargets(1, nullRTVs.data(), nullptr);
		}

		// SSR pass
		if ((viewIdx == RendererView::Final || viewIdx == RendererView::SSRTrace) && IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(SSRRenderPassName);

			pass->BindSharedResources(gfx);
			pass->Execute(gfx); // setup binds

			static SSR_CB ssrCB;
			ssrCB.debugViewStep = (frameCt / 30u) % 20u;
			pSSR_CB->Update(gfx, ssrCB);

			pSSRKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// FXAA pass
		if (Config::AAType == Config::AAType::FXAA && viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::FXAA))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(FXAARenderPassName);

			pass->BindSharedResources(gfx);
			pass->Execute(gfx); // setup binds

			static FXAA_CB fxaaCB;
			fxaaCB.minThreshold = 0.1f;
			fxaaCB.maxThreshold = 2.f;
			fxaaCB.edgeSharpness = 0.25f;
			fxaaCB.padding = 0.f;
			pFXAA_CB->Update(gfx, fxaaCB);

			pFXAAKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// Dither pass
		if (viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Dither))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(DitherRenderPassName);

			pass->BindSharedResources(gfx);
			pass->Execute(gfx); // setup binds

			static DitherCB ditherCB;
			ditherCB.shadowDither = 0.15f;
			ditherCB.midDither = 0.04f;
			pDitherCB->Update(gfx, ditherCB);

			pDitherKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// Tonemapping pass
		if (viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Tonemapping))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(TonemappingRenderPassName);

			pass->BindSharedResources(gfx);
			pass->Execute(gfx); // setup binds

			pTonemappingKernel->Dispatch(gfx, *pass, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// Final blit
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(FinalBlitRenderPassName);
			const auto fsPass = static_cast<FullscreenPass*>(pass.get());

			// todo: remove this - for now it's needed to clear SRVs
			gfx.GetContext()->ClearState();

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			// Debug view overrides: (do this here so it can be changed dynamically later)
			switch (viewIdx)
			{
			case RendererView::Final:
				fsPass->SetInputTarget(pFinalBlitInputIs0 ? pCameraColor0 : pCameraColor1);
				break;
			case RendererView::TiledLighting:
				fsPass->SetInputTarget(pDebugTiledLightingCS);
				break;
			case RendererView::SSRTrace:
				fsPass->SetInputTarget(pFinalBlitInputIs0 ? pCameraColor0 : pCameraColor1);
				break;
			}

			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());
			gfx.GetContext()->OMSetRenderTargets(1u, gfx.pBackBufferView.GetAddressOf(), gfx.pDepthStencil->GetView().Get());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

	}

	void Renderer::DrawImguiControlWindow()
	{
		if (ImGui::Begin("Renderer"))
		{
			static ImVec2 buttonSize = { 110, 25 };
			static ImVec2 featureButtonSize = { 70, 18 };

			// Select view
			ImGui::Text("View:");
			int newViewIdx = (int)viewIdx;
			newViewIdx = DrawSelectableButtonInArray(0, "Final", newViewIdx, buttonSize, false);
			newViewIdx = DrawSelectableButtonInArray(1, "Tiled Lighting", newViewIdx, buttonSize, true);
			newViewIdx = DrawSelectableButtonInArray(2, "SSR Trace", newViewIdx, buttonSize, true);
			viewIdx = (RendererView)newViewIdx;

			// Toggle features
			ImGui::Text("Features:");

			rendererFeatureEnabled[(int)RendererFeature::Shadows] = DrawToggleOnOffButton(0, "Shadows", rendererFeatureEnabled[(int)RendererFeature::Shadows], featureButtonSize);
			rendererFeatureEnabled[(int)RendererFeature::FXAA] = DrawToggleOnOffButton(1, "FXAA", rendererFeatureEnabled[(int)RendererFeature::FXAA], featureButtonSize);
			rendererFeatureEnabled[(int)RendererFeature::HZBSSR] = DrawToggleOnOffButton(2, "Hi-Z SSR", rendererFeatureEnabled[(int)RendererFeature::HZBSSR], featureButtonSize);
			rendererFeatureEnabled[(int)RendererFeature::Dither] = DrawToggleOnOffButton(3, "Dither", rendererFeatureEnabled[(int)RendererFeature::Dither], featureButtonSize);
			rendererFeatureEnabled[(int)RendererFeature::Tonemapping] = DrawToggleOnOffButton(4, "Tonemapping", rendererFeatureEnabled[(int)RendererFeature::Tonemapping], featureButtonSize);
			
		}
		ImGui::End();
	}

	void Renderer::Reset()
	{
		for (auto& p : pRenderPasses)
		{
			p.second->Reset();
		}
	}

	const std::unique_ptr<RenderPass>& Renderer::GetRenderPass(const std::string name)
	{
		const std::unique_ptr<RenderPass>& pass = pRenderPasses[name];
		return pass;
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

	bool Renderer::IsFeatureEnabled(RendererFeature feature) const
	{
		return rendererFeatureEnabled[(int)feature];
	}
}