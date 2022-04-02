#include "pch.h"
#include "Renderer.h"
#include <array>
#include "DepthStencilState.h"
#include "GraphicsDevice.h"
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
	Renderer::Renderer(GraphicsDevice& gfx, std::shared_ptr<LightManager> pLightManager, std::shared_ptr<RendererList> pRendererList)
		: pRendererList(pRendererList), pLightManager(pLightManager)
	{
		//
		// Debug stuff
		//
		rendererFeatureEnabled.resize(RendererFeature::COUNT, true); // enable all features by default
		//rendererFeatureEnabled[RendererFeature::HZBSSR] = false;

		//
		// Components
		//
		pVisibleRendererList = std::make_unique<RendererList>(pRendererList);

		//
		// Texture assets and samplers
		//
		pDitherTexture = std::dynamic_pointer_cast<Texture>(Texture::Resolve(gfx, "Assets\\Textures\\Dither8x8.png"));
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
		pNormalRoughReflectivityTarget->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pHiZBufferTarget = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16_UNORM, 8u);
		pHiZBufferTarget->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pBloomPyramid = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT, 8u);
		pBloomPyramid->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pSpecularLighting = std::make_shared<RenderTexture>(gfx);
		pSpecularLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDiffuseLighting = std::make_shared<RenderTexture>(gfx);
		pDiffuseLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pCameraColor0 = std::make_shared<RenderTexture>(gfx);
		pCameraColor0->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pCameraColor1 = std::make_shared<RenderTexture>(gfx);
		pCameraColor1->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDebugTiledLighting = std::make_shared<RenderTexture>(gfx);
		pDebugTiledLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDebugClusteredLighting = std::make_shared<RenderTexture>(gfx);
		pDebugClusteredLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		pDebugSSR = std::make_shared<RenderTexture>(gfx);
		pDebugSSR->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		//
		// Buffers
		//
		pPerFrameCB = std::make_unique<ConstantBuffer<PerFrameCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pTransformationCB = std::make_unique<ConstantBuffer<GlobalTransformCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pPerCameraCB = std::make_unique<ConstantBuffer<PerCameraCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pHiZCreationCB = std::make_unique<ConstantBuffer<HiZCreationCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pClusteredLightingCB = std::make_unique<ConstantBuffer<ClusteredLightingCB>>(gfx, D3D11_USAGE_DYNAMIC);

		pFXAA_CB = std::make_unique<ConstantBuffer<FXAA_CB>>(gfx, D3D11_USAGE_DYNAMIC);
		pSSR_CB = std::make_unique<ConstantBuffer<SSR_CB>>(gfx, D3D11_USAGE_DYNAMIC);
		pSSR_DebugData = std::make_unique<StructuredBuffer<int>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 10u);
		pDitherCB = std::make_unique<ConstantBuffer<DitherCB>>(gfx, D3D11_USAGE_DYNAMIC);

		//
		// Compute shaders
		//
		pHiZDepthCopyKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\HiZDepthCopy.cso"), std::string("CSMain")));
		pHiZCreateMipKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\HiZCreateMip.cso"), std::string("CSMain")));
		pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingCompute.cso"), std::string("CSMain")));
		pClusteredLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\ClusteredLightingCompute.cso"), std::string("CSMain")));
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
		CreateRenderPass(ClusteredLightingPassName);
		CreateRenderPass(OpaqueRenderPassName);
		CreateRenderPass(BlurPyramidPassName);
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

	void Renderer::SetupRenderPassDependencies(GraphicsDevice& gfx)
	{
		bool cameraOutSlot0 = true;

		GetRenderPass(PerCameraPassName)->
			ClearBinds()
			.CSSetCB(RenderSlots::CS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_LightInputCB, pLightManager->GetLightInputCB()->GetD3DBuffer())
			.CSSetSRV(RenderSlots::CS_LightDataSRV, pLightManager->GetLightDataSRV())
			.CSSetSRV(RenderSlots::CS_LightShadowDataSRV, pLightManager->GetShadowDataSRV())
			.VSSetCB(RenderSlots::VS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerFrameCB, pPerFrameCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_TransformationCB, pTransformationCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerCameraCB, pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_LightInputCB, pLightManager->GetLightInputCB()->GetD3DBuffer());

		GetRenderPass(DepthPrepassName)->
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(HiZPassName)->
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pHiZBufferTarget->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, nullptr)
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, pHiZCreationCB->GetD3DBuffer());

		GetRenderPass(ShadowPassName)->
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_FRONT)).SetupRSBinding(); // Reduce shadow acne w/ front face culling during shadow pass

		GetRenderPass(GBufferRenderPassName)->
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(TiledLightingPassName)->
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pNormalRoughReflectivityTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, pHiZBufferTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, pLightManager->GetShadowAtlas()->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, pDitherTexture->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pSpecularLighting->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, pDiffuseLighting->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, pDebugTiledLighting->GetUAV())
			.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, pShadowSampler->GetD3DSampler());

		GetRenderPass(ClusteredLightingPassName)->
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, pHiZBufferTarget->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pLightManager->GetClusteredIndices()->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, pDebugClusteredLighting->GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, pClusteredLightingCB->GetD3DBuffer());

		GetRenderPass(OpaqueRenderPassName)->
			ClearBinds()
			.PSSetSRV(RenderSlots::PS_FreeSRV + 0u, pSpecularLighting->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 1u, pDiffuseLighting->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 2u, pLightManager->GetClusteredIndices()->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 3u, pDitherTexture->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 4u, pLightManager->GetLightDataSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 5u, pLightManager->GetShadowDataSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 6u, pLightManager->GetShadowAtlas()->GetSRV())
			.PSSetCB(RenderSlots::PS_FreeCB + 0u, pClusteredLightingCB->GetD3DBuffer())
			.PSSetSPL(RenderSlots::PS_FreeSPL + 0u, pShadowSampler->GetD3DSampler());

		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(SSRRenderPassName)->
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, pNormalRoughReflectivityTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, gfx.GetDepthStencilTarget()->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, pHiZBufferTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, pDitherTexture->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, pSSR_DebugData->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, pDebugSSR->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, pSSR_CB->GetD3DBuffer())
				.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, pSampler_ClampedBilinear->GetD3DSampler());
		}
		
		if (IsFeatureEnabled(RendererFeature::FXAA))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(FXAARenderPassName)->
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, pFXAA_CB->GetD3DBuffer())
				.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, pSampler_ClampedBilinear->GetD3DSampler());
		}

		if (IsFeatureEnabled(RendererFeature::Dither))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(DitherRenderPassName)->
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, pDitherTexture->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, pDitherCB->GetD3DBuffer());
		}

		if (IsFeatureEnabled(RendererFeature::Tonemapping))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? pCameraColor0 : pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? pCameraColor1 : pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(TonemappingRenderPassName)->
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV());
		}

		// Determine what to use for final blit
		pFinalBlitInputIs0 = cameraOutSlot0;
	}

	void Renderer::AcceptDrawCall(DrawCall job, std::string targetPass)
	{
		pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Renderer::Execute(GraphicsDevice & gfx, const std::unique_ptr<Camera>& cam, float timeElapsed, UINT pixelSelectionX, UINT pixelSelectionY)
	{
		auto context = gfx.GetContext();

		context->ClearState();

		static int frameCt = 0;
		frameCt++;

		// Shadow + lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(ShadowPassName);

			pass->BindSharedResources(gfx);

			static ShadowPassContext context(gfx, cam, *this, pass, pTransformationCB, nullptr);
			context.Update();

			pLightManager->CullLights(gfx, cam, IsFeatureEnabled(RendererFeature::Shadows)); // changes the SRV, which will be bound in per-frame binds
			if (IsFeatureEnabled(RendererFeature::Shadows))
			{
				pLightManager->RenderShadows(context);
			}

			pass->UnbindSharedResources(gfx);
		}

		// Submit draw calls
		{
			static DrawContext drawContext(*this, std::move(std::vector<std::string> { DepthPrepassName, GBufferRenderPassName, OpaqueRenderPassName }));
			drawContext.viewMatrix = cam->GetViewMatrix();
			drawContext.projMatrix = cam->GetProjectionMatrix();

			// todo: filter by render passes too
			pVisibleRendererList->Filter(cam->GetFrustumWS(), RendererList::RendererSorting::BackToFront);
			pVisibleRendererList->SubmitDrawCalls(drawContext);
		}

		// Early frame calculations
		{
			static PerFrameCB perFrameCB;
			ZeroMemory(&perFrameCB, sizeof(perFrameCB));
			perFrameCB.pixelSelection = { pixelSelectionX, pixelSelectionY, (UINT)pixelIteration, 0u };
			perFrameCB.time = { timeElapsed / 20.f, timeElapsed, timeElapsed * 2.f, timeElapsed * 3.f };
			perFrameCB.sinTime = { std::sin(timeElapsed / 8.f), std::sin(timeElapsed / 4.f), std::sin(timeElapsed / 2.f), std::sin(timeElapsed) };
			perFrameCB.cosTime = { std::cos(timeElapsed / 8.f), std::cos(timeElapsed / 4.f), std::cos(timeElapsed / 2.f), std::cos(timeElapsed) };
			pPerFrameCB->Update(gfx, perFrameCB);

			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = cam->GetViewMatrix();
			transformationCB.projMatrix = cam->GetProjectionMatrix();
			transformationCB.viewProjMatrix = transformationCB.projMatrix * transformationCB.viewMatrix;
			transformationCB.invViewMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewMatrix);
			transformationCB.invProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.projMatrix);
			transformationCB.invViewProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewProjMatrix);
			pTransformationCB->Update(gfx, transformationCB);

			const float farNearRatio = cam->GetFarClipPlane() / cam->GetNearClipPlane();

			static PerCameraCB perCameraCB;
			ZeroMemory(&perCameraCB, sizeof(perCameraCB));
			perCameraCB.projectionParams = dx::XMVectorSet(1.f, cam->GetNearClipPlane(), cam->GetFarClipPlane(), 1.f / cam->GetFarClipPlane());
			perCameraCB.screenParams = dx::XMVectorSet((float)gfx.GetScreenWidth(), (float)gfx.GetScreenHeight(), 1.0f / gfx.GetScreenWidth(), 1.0f / gfx.GetScreenHeight());
			perCameraCB.zBufferParams = dx::XMVectorSet(1.f - farNearRatio, farNearRatio, 1.f / cam->GetFarClipPlane() - 1.f / cam->GetNearClipPlane(), 1.f / cam->GetNearClipPlane());
			perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
			perCameraCB.frustumCornerDataVS = cam->GetFrustumCornersVS();
			perCameraCB.cameraPositionWS = cam->GetPositionWS();

			// todo: move elsewhere, and only calculate when FOV or resolution changes?
			float fClustersZ = (float)pLightManager->GetClusterDimensionZ();
			float logFarOverNear = std::log2f(farNearRatio);
			perCameraCB.clusterPrecalc = dx::XMVectorSet(fClustersZ / logFarOverNear, -(fClustersZ * std::log2f(cam->GetNearClipPlane()) / logFarOverNear), 0.f, 0.f);

			// This is used when calculating cluster.xy from NDC
			// These calculations turn it into a single [MAD] operation
			const float invClusterDim = 1.f / 16.f;
			perCameraCB.clusterXYRemap = dx::XMVectorSet(
				gfx.GetScreenWidth() * 0.5f * invClusterDim,
				gfx.GetScreenHeight() * 0.5f * invClusterDim,
				0.f,
				0.f
			);
			pPerCameraCB->Update(gfx, perCameraCB);
		}

		// Per-frame and per-camera binds
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(PerCameraPassName);
			pass->BindSharedResources(gfx);
		}

		// Early Z pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(DepthPrepassName);

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			gfx.GetDepthStencilTarget()->Clear(gfx);

			gfx.SetDepthOnlyRenderTarget();
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

		// Hi-Z buffer pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(HiZPassName);

			gfx.ClearRenderTargets(); // need in order to access depth
			pass->BindSharedResources(gfx);

			const UINT screenWidth = (UINT)gfx.GetScreenWidth();
			const UINT screenHeight = (UINT)gfx.GetScreenHeight();

			static HiZCreationCB hiZCreationCB;
			hiZCreationCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			pHiZCreationCB->Update(gfx, hiZCreationCB);

			// Copy from depth-stencil
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV, 1u, pHiZBufferTarget->GetUAV(0u).GetAddressOf(), nullptr);
			pHiZDepthCopyKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			// Create other mips
			for (UINT mip = 1u; mip < 8u; ++mip)
			{
				const UINT dstWidth = screenWidth >> mip;
				const UINT dstHeight = screenHeight >> mip;

				if (dstWidth == 0u || dstHeight == 0u)
					continue;

				hiZCreationCB.resolutionSrcDst = { dstWidth << 1u, dstHeight << 1u, dstWidth, dstHeight };
				pHiZCreationCB->Update(gfx, hiZCreationCB);

				// Bind mip slice views as UAVs
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, pass->pNullUAVs.data(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, pHiZBufferTarget->GetUAV(mip - 1u).GetAddressOf(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, pHiZBufferTarget->GetUAV(mip).GetAddressOf(), nullptr);

				pHiZCreateMipKernel->Dispatch(gfx, dstWidth, dstHeight, 1u);
			}

			pass->UnbindSharedResources(gfx);
		}

		// Normal-rough-reflectivity pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(GBufferRenderPassName);

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			pNormalRoughReflectivityTarget->ClearRenderTarget(context.Get(), 1.f, 0.f, 0.f, 1.f);

			gfx.SetRenderTarget(pNormalRoughReflectivityTarget->GetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);

			gfx.ClearRenderTargets();

			pass->UnbindSharedResources(gfx);
		}

		// Tiled lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(TiledLightingPassName);
			pass->BindSharedResources(gfx);

			pTiledLightingKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// Clustered lighting pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(ClusteredLightingPassName);
			pass->BindSharedResources(gfx);

			static ClusteredLightingCB clusteredLightingCB;
			clusteredLightingCB.groupResolutions = { pLightManager->GetClusterDimensionX(), pLightManager->GetClusterDimensionY(), pLightManager->GetClusterDimensionZ(), 0u };
			pClusteredLightingCB->Update(gfx, clusteredLightingCB);

			pClusteredLightingKernel->Dispatch(gfx, pLightManager->GetClusterDimensionX(), pLightManager->GetClusterDimensionY(), pLightManager->GetClusterDimensionZ());

			pass->UnbindSharedResources(gfx);
		}

		// Opaque pass
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(OpaqueRenderPassName);
			pass->BindSharedResources(gfx);

			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);

			pCameraColor0->ClearRenderTarget(context.Get(), 0.f, 0.f, 0.f, 0.f);
			pCameraColor0->BindAsTarget(gfx, gfx.GetDepthStencilTarget()->GetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);

			gfx.ClearRenderTargets();
		}

		// Blur pyramid
		{
			/*const std::unique_ptr<RenderPass>& pass = GetRenderPass(BlurPyramidPassName);
			pass->BindSharedResources(gfx);

			UINT screenWidth = (UINT)gfx.GetScreenWidth();
			UINT screenHeight = (UINT)gfx.GetScreenHeight();

			static BlurPyramidCreationCB blurPyramidCreationCB;
			blurPyramidCreationCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			pBlurPyramidCreationCB->Update(gfx, blurPyramidCreationCB);

			// Downsample
			for (UINT mip = 0u; mip < 8u; ++mip)
			{
				UINT dstWidth = screenWidth >> (mip + 1u);
				UINT dstHeight = screenHeight >> (mip + 1u);

				if (dstWidth == 0u || dstHeight == 0u)
					continue;

				hiZCreationCB.resolutionSrcDst = { dstWidth << 1u, dstHeight << 1u, dstWidth, dstHeight };
				pHiZCreationCB->Update(gfx, hiZCreationCB);

				// Bind mip slice views as UAVs
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, pass->pNullUAVs.data(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, pHiZBufferTarget->GetUAV(mip - 1u).GetAddressOf(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, pHiZBufferTarget->GetUAV(mip).GetAddressOf(), nullptr);

				pHiZCreateMipKernel->Dispatch(gfx, dstWidth, dstHeight, 1u);
			}

			pass->UnbindSharedResources(gfx);*/
		}

		// SSR pass
		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(SSRRenderPassName);
			pass->BindSharedResources(gfx);

			static SSR_CB ssrCB;
			ssrCB.debugViewStep = (frameCt / 20u) % 25u;
			pSSR_CB->Update(gfx, ssrCB);

			pSSRKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// FXAA pass
		if (Config::AAType == Config::AAType::FXAA && IsFeatureEnabled(RendererFeature::FXAA))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(FXAARenderPassName);
			pass->BindSharedResources(gfx);

			static FXAA_CB fxaaCB;
			fxaaCB.minThreshold = 0.1f;
			fxaaCB.maxThreshold = 2.f;
			fxaaCB.edgeSharpness = 0.25f;
			fxaaCB.padding = 0.f;
			pFXAA_CB->Update(gfx, fxaaCB);

			pFXAAKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// Dither pass
		if (viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Dither))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(DitherRenderPassName);
			pass->BindSharedResources(gfx);

			static DitherCB ditherCB;
			ditherCB.shadowDither = 0.15f;
			ditherCB.midDither = 0.04f;
			pDitherCB->Update(gfx, ditherCB);

			pDitherKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// Tonemapping pass
		if (viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Tonemapping))
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(TonemappingRenderPassName);
			pass->BindSharedResources(gfx);

			pTonemappingKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass->UnbindSharedResources(gfx);
		}

		// Final blit
		{
			const std::unique_ptr<RenderPass>& pass = GetRenderPass(FinalBlitRenderPassName);
			const auto fsPass = static_cast<FullscreenPass*>(pass.get());

			// todo: remove this - for now it's needed to clear SRVs
			context->ClearState();

			pass->BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			// Debug view overrides: (do this here so it can be changed dynamically later)
			switch (viewIdx)
			{
			case RendererView::Final:
				fsPass->SetInputTarget(pFinalBlitInputIs0 ? pCameraColor0 : pCameraColor1);
				break;
			case RendererView::TiledLighting:
				fsPass->SetInputTarget(pDebugTiledLighting);
				break;
			case RendererView::ClusteredLighting:
				fsPass->SetInputTarget(pDebugClusteredLighting);
				break;
			case RendererView::SSRTrace:
				fsPass->SetInputTarget(pDebugSSR);
				break;
			}

			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());
			gfx.SetRenderTarget(gfx.GetBackBufferView());

			pass->Execute(gfx);
			pass->UnbindSharedResources(gfx);
		}

	}

	void Renderer::DrawImguiControlWindow(GraphicsDevice& gfx)
	{
		if (ImGui::Begin("Renderer"))
		{
			static ImVec2 buttonSize = { 110, 25 };
			static ImVec2 featureButtonSize = { 70, 18 };

			bool changed = false;

			// Select view
			ImGui::Text("View:");
			int newViewIdx = (int)viewIdx;
			newViewIdx = DrawSelectableButtonInArray(0, "Final", newViewIdx, buttonSize, changed, false);
			newViewIdx = DrawSelectableButtonInArray(1, "Tiled Lighting", newViewIdx, buttonSize, changed, true);
			newViewIdx = DrawSelectableButtonInArray(2, "Clustered Lighting", newViewIdx, buttonSize, changed, true);
			newViewIdx = DrawSelectableButtonInArray(3, "SSR Trace", newViewIdx, buttonSize, changed, true);
			viewIdx = (RendererView)newViewIdx;

			int _pixelIteration = pixelIteration;
			ImGui::SliderInt("Iteration:", &_pixelIteration, 0, 100);
			pixelIteration = _pixelIteration;

			// Toggle features
			ImGui::Text("Features:");

			//static ImGuiTableFlags flags = ImGuiTableFlags_PreciseWidths;
			ImGui::BeginTable("FeatureTable", 2);
			rendererFeatureEnabled[(int)RendererFeature::Shadows] = DrawToggleOnOffButton(0, "Shadows", rendererFeatureEnabled[(int)RendererFeature::Shadows], featureButtonSize, changed);
			rendererFeatureEnabled[(int)RendererFeature::FXAA] = DrawToggleOnOffButton(1, "FXAA", rendererFeatureEnabled[(int)RendererFeature::FXAA], featureButtonSize, changed);
			rendererFeatureEnabled[(int)RendererFeature::HZBSSR] = DrawToggleOnOffButton(2, "Hi-Z SSR", rendererFeatureEnabled[(int)RendererFeature::HZBSSR], featureButtonSize, changed);
			rendererFeatureEnabled[(int)RendererFeature::Dither] = DrawToggleOnOffButton(3, "Dither", rendererFeatureEnabled[(int)RendererFeature::Dither], featureButtonSize, changed);
			rendererFeatureEnabled[(int)RendererFeature::Tonemapping] = DrawToggleOnOffButton(4, "Tonemapping", rendererFeatureEnabled[(int)RendererFeature::Tonemapping], featureButtonSize, changed);
			ImGui::EndTable();
			
			if (changed)
			{
				// Need to re-route inputs and outputs
				SetupRenderPassDependencies(gfx);
			}

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