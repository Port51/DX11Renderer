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
#include "RenderStats.h"

namespace gfx
{
	Renderer::Renderer(const GraphicsDevice& gfx, std::shared_ptr<LightManager> pLightManager, std::shared_ptr<RendererList> pRendererList)
		: m_pRendererList(pRendererList), m_pLightManager(pLightManager)
	{
		//
		// Debug stuff
		//
		m_rendererFeatureEnabled.resize(RendererFeature::COUNT, true); // enable all features by default
		//rendererFeatureEnabled[RendererFeature::HZBSSR] = false;

		//
		// Components
		//
		m_pVisibleRendererList = std::make_unique<RendererList>(pRendererList);

		//
		// Texture assets and samplers
		//
		m_pDitherTexture = std::dynamic_pointer_cast<Texture>(Texture::Resolve(gfx, "Assets\\Textures\\Dither8x8.png"));
		m_pClampedBilinearSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);
		
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

		m_pShadowSampler = std::make_shared<Sampler>(gfx, shadowSamplerDesc);

		//
		// Render targets
		//
		m_pNormalRoughReflectivityTarget = std::make_shared<RenderTexture>(gfx);
		m_pNormalRoughReflectivityTarget->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pHiZBufferTarget = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16_UNORM, 8u);
		m_pHiZBufferTarget->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pBloomPyramid = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT, 8u);
		m_pBloomPyramid->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pSpecularLighting = std::make_shared<RenderTexture>(gfx);
		m_pSpecularLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pDiffuseLighting = std::make_shared<RenderTexture>(gfx);
		m_pDiffuseLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pCameraColor0 = std::make_shared<RenderTexture>(gfx);
		m_pCameraColor0->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pCameraColor1 = std::make_shared<RenderTexture>(gfx);
		m_pCameraColor1->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pDebugTiledLighting = std::make_shared<RenderTexture>(gfx);
		m_pDebugTiledLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pDebugClusteredLighting = std::make_shared<RenderTexture>(gfx);
		m_pDebugClusteredLighting->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		m_pDebugSSR = std::make_shared<RenderTexture>(gfx);
		m_pDebugSSR->Init(gfx.GetAdapter(), gfx.GetScreenWidth(), gfx.GetScreenHeight());

		//
		// Buffers
		//
		m_pPerFrameCB = std::make_unique<ConstantBuffer<PerFrameCB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pTransformationCB = std::make_unique<ConstantBuffer<GlobalTransformCB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pPerCameraCB = std::make_unique<ConstantBuffer<PerCameraCB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pHiZCreationCB = std::make_unique<ConstantBuffer<HiZCreationCB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pClusteredLightingCB = std::make_unique<ConstantBuffer<ClusteredLightingCB>>(gfx, D3D11_USAGE_DYNAMIC);

		m_pFXAA_CB = std::make_unique<ConstantBuffer<FXAA_CB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pSSR_CB = std::make_unique<ConstantBuffer<SSR_CB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pSSR_DebugData = std::make_unique<StructuredBuffer<int>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 10u);
		m_pDitherCB = std::make_unique<ConstantBuffer<DitherCB>>(gfx, D3D11_USAGE_DYNAMIC);

		//
		// Compute shaders
		//
		m_pHiZDepthCopyKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\HiZDepthCopy.cso"), std::string("CSMain")));
		m_pHiZCreateMipKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\HiZCreateMip.cso"), std::string("CSMain")));
		m_pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\TiledLightingCompute.cso"), std::string("CSMain")));
		m_pClusteredLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\ClusteredLightingCompute.cso"), std::string("CSMain")));
		m_pSSRKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\SSR.cso"), std::string("CSMain")));
		m_pFXAAKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\FXAA.cso"), std::string("CSMain")));
		m_pDitherKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\Dither.cso"), std::string("CSMain")));
		m_pTonemappingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, std::string("Assets\\Built\\Shaders\\Tonemapping.cso"), std::string("CSMain")));

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

	void Renderer::SetupRenderPassDependencies(const GraphicsDevice& gfx)
	{
		bool cameraOutSlot0 = true;

		GetRenderPass(PerCameraPassName).
			ClearBinds()
			.CSSetCB(RenderSlots::CS_PerFrameCB, m_pPerFrameCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_TransformationCB, m_pTransformationCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_PerCameraCB, m_pPerCameraCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_LightInputCB, m_pLightManager->GetLightInputCB().GetD3DBuffer())
			.CSSetSRV(RenderSlots::CS_LightDataSRV, m_pLightManager->GetLightDataSRV())
			.CSSetSRV(RenderSlots::CS_LightShadowDataSRV, m_pLightManager->GetShadowDataSRV())
			.VSSetCB(RenderSlots::VS_PerFrameCB, m_pPerFrameCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_TransformationCB, m_pTransformationCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_PerCameraCB, m_pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerFrameCB, m_pPerFrameCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_TransformationCB, m_pTransformationCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerCameraCB, m_pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_LightInputCB, m_pLightManager->GetLightInputCB().GetD3DBuffer());

		GetRenderPass(DepthPrepassName).
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(HiZPassName).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pHiZBufferTarget->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, nullptr)
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pHiZCreationCB->GetD3DBuffer());

		GetRenderPass(ShadowPassName).
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_FRONT)).SetupRSBinding(); // Reduce shadow acne w/ front face culling during shadow pass

		GetRenderPass(GBufferRenderPassName).
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(TiledLightingPassName).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, m_pNormalRoughReflectivityTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pHiZBufferTarget->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, m_pLightManager->GetShadowAtlas().GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, m_pDitherTexture->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pSpecularLighting->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDiffuseLighting->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, m_pDebugTiledLighting->GetUAV())
			.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, m_pShadowSampler->GetD3DSampler());

		GetRenderPass(ClusteredLightingPassName).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pHiZBufferTarget->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pLightManager->GetClusteredIndices().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDebugClusteredLighting->GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pClusteredLightingCB->GetD3DBuffer());

		GetRenderPass(OpaqueRenderPassName).
			ClearBinds()
			.PSSetSRV(RenderSlots::PS_FreeSRV + 0u, m_pSpecularLighting->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 1u, m_pDiffuseLighting->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 2u, m_pLightManager->GetClusteredIndices().GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 3u, m_pDitherTexture->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 4u, m_pLightManager->GetLightDataSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 5u, m_pLightManager->GetShadowDataSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 6u, m_pLightManager->GetShadowAtlas().GetSRV())
			.PSSetCB(RenderSlots::PS_FreeCB + 0u, m_pClusteredLightingCB->GetD3DBuffer())
			.PSSetSPL(RenderSlots::PS_FreeSPL + 0u, m_pShadowSampler->GetD3DSampler());

		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(SSRRenderPassName).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, m_pNormalRoughReflectivityTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, gfx.GetDepthStencilTarget()->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, m_pHiZBufferTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, m_pDitherTexture->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pSSR_DebugData->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, m_pDebugSSR->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pSSR_CB->GetD3DBuffer())
				.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, m_pClampedBilinearSampler->GetD3DSampler());
		}
		
		if (IsFeatureEnabled(RendererFeature::FXAA))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(FXAARenderPassName).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pFXAA_CB->GetD3DBuffer())
				.CSSetSPL(RenderSlots::CS_FreeSPL + 0u, m_pClampedBilinearSampler->GetD3DSampler());
		}

		if (IsFeatureEnabled(RendererFeature::Dither))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(DitherRenderPassName).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pDitherTexture->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pDitherCB->GetD3DBuffer());
		}

		if (IsFeatureEnabled(RendererFeature::Tonemapping))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(TonemappingRenderPassName).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV());
		}

		// Determine what to use for final blit
		m_pFinalBlitInputIsIndex0 = cameraOutSlot0;
	}

	void Renderer::AcceptDrawCall(DrawCall job, std::string targetPass)
	{
		m_pRenderPasses[targetPass]->EnqueueJob(job);
	}

	void Renderer::Execute(GraphicsDevice& gfx, const Camera& camera, float timeElapsed, UINT pixelSelectionX, UINT pixelSelectionY)
	{
		auto context = gfx.GetContext();

		gfx.GetRenderStats().StartFrame();
		context->ClearState();

		static int frameCt = 0;
		frameCt++;

		// Shadow + lighting pass
		{
			RenderPass& pass = GetRenderPass(ShadowPassName);

			pass.BindSharedResources(gfx);

			static ShadowPassContext context(gfx, camera, *this, pass, *m_pTransformationCB.get(), nullptr);
			context.Update();

			m_pLightManager->CullLightsAndShadows(gfx, camera, IsFeatureEnabled(RendererFeature::Shadows)); // changes the SRV, which will be bound in per-frame binds
			if (IsFeatureEnabled(RendererFeature::Shadows))
			{
				m_pLightManager->RenderShadows(context);
			}

			pass.UnbindSharedResources(gfx);
		}

		// Submit draw calls
		{
			static DrawContext drawContext(*this, std::move(std::vector<std::string> { DepthPrepassName, GBufferRenderPassName, OpaqueRenderPassName }));
			drawContext.viewMatrix = camera.GetViewMatrix();
			drawContext.projMatrix = camera.GetProjectionMatrix();

			// todo: filter by render passes too
			m_pVisibleRendererList->Filter(gfx, camera.GetFrustumWS(), RendererList::RendererSortingType::StateThenBackToFront, camera.GetPositionWS(), camera.GetForwardWS(), camera.GetFarClipPlane());
			m_pVisibleRendererList->SubmitDrawCalls(drawContext);
		}

		// Early frame calculations
		{
			static PerFrameCB perFrameCB;
			ZeroMemory(&perFrameCB, sizeof(perFrameCB));
			perFrameCB.pixelSelection = { pixelSelectionX, pixelSelectionY, (UINT)m_pixelIteration, 0u };
			perFrameCB.time = { timeElapsed / 20.f, timeElapsed, timeElapsed * 2.f, timeElapsed * 3.f };
			perFrameCB.sinTime = { std::sin(timeElapsed / 8.f), std::sin(timeElapsed / 4.f), std::sin(timeElapsed / 2.f), std::sin(timeElapsed) };
			perFrameCB.cosTime = { std::cos(timeElapsed / 8.f), std::cos(timeElapsed / 4.f), std::cos(timeElapsed / 2.f), std::cos(timeElapsed) };
			m_pPerFrameCB->Update(gfx, perFrameCB);

			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = camera.GetViewMatrix();
			transformationCB.projMatrix = camera.GetProjectionMatrix();
			transformationCB.viewProjMatrix = transformationCB.projMatrix * transformationCB.viewMatrix;
			transformationCB.invViewMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewMatrix);
			transformationCB.invProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.projMatrix);
			transformationCB.invViewProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewProjMatrix);
			m_pTransformationCB->Update(gfx, transformationCB);

			const float farNearRatio = camera.GetFarClipPlane() / camera.GetNearClipPlane();

			static PerCameraCB perCameraCB;
			ZeroMemory(&perCameraCB, sizeof(perCameraCB));
			perCameraCB.projectionParams = dx::XMVectorSet(1.f, camera.GetNearClipPlane(), camera.GetFarClipPlane(), 1.f / camera.GetFarClipPlane());
			perCameraCB.screenParams = dx::XMVectorSet((float)gfx.GetScreenWidth(), (float)gfx.GetScreenHeight(), 1.0f / gfx.GetScreenWidth(), 1.0f / gfx.GetScreenHeight());
			perCameraCB.zBufferParams = dx::XMVectorSet(1.f - farNearRatio, farNearRatio, 1.f / camera.GetFarClipPlane() - 1.f / camera.GetNearClipPlane(), 1.f / camera.GetNearClipPlane());
			perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
			perCameraCB.frustumCornerDataVS = camera.GetFrustumCornersVS();
			perCameraCB.cameraPositionWS = camera.GetPositionWS();

			// todo: move elsewhere, and only calculate when FOV or resolution changes?
			float fClustersZ = (float)m_pLightManager->GetClusterDimensionZ();
			float logFarOverNear = std::log2f(farNearRatio);
			perCameraCB.clusterPrecalc = dx::XMVectorSet(fClustersZ / logFarOverNear, -(fClustersZ * std::log2f(camera.GetNearClipPlane()) / logFarOverNear), 0.f, 0.f);

			// This is used when calculating cluster.xy from NDC
			// These calculations turn it into a single [MAD] operation
			const float invClusterDim = 1.f / 16.f;
			perCameraCB.clusterXYRemap = dx::XMVectorSet(
				gfx.GetScreenWidth() * 0.5f * invClusterDim,
				gfx.GetScreenHeight() * 0.5f * invClusterDim,
				0.f,
				0.f
			);
			m_pPerCameraCB->Update(gfx, perCameraCB);
		}

		// Per-frame and per-camera binds
		{
			const RenderPass& pass = GetRenderPass(PerCameraPassName);
			pass.BindSharedResources(gfx);
		}

		// Early Z pass
		{
			const RenderPass& pass = GetRenderPass(DepthPrepassName);

			pass.BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);
			gfx.GetDepthStencilTarget()->Clear(gfx);

			gfx.SetDepthOnlyRenderTarget();
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass.Execute(gfx);
			pass.UnbindSharedResources(gfx);
		}

		// Hi-Z buffer pass
		{
			const RenderPass& pass = GetRenderPass(HiZPassName);

			gfx.ClearRenderTargets(); // need in order to access depth
			pass.BindSharedResources(gfx);

			const UINT screenWidth = (UINT)gfx.GetScreenWidth();
			const UINT screenHeight = (UINT)gfx.GetScreenHeight();

			static HiZCreationCB hiZCreationCB;
			hiZCreationCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			m_pHiZCreationCB->Update(gfx, hiZCreationCB);

			// Copy from depth-stencil
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV, 1u, m_pHiZBufferTarget->GetUAV(0u).GetAddressOf(), nullptr);
			m_pHiZDepthCopyKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			// Create other mips
			for (UINT mip = 1u; mip < 8u; ++mip)
			{
				const UINT dstWidth = screenWidth >> mip;
				const UINT dstHeight = screenHeight >> mip;

				if (dstWidth == 0u || dstHeight == 0u)
					continue;

				hiZCreationCB.resolutionSrcDst = { dstWidth << 1u, dstHeight << 1u, dstWidth, dstHeight };
				m_pHiZCreationCB->Update(gfx, hiZCreationCB);

				// Bind mip slice views as UAVs
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, pass.m_pNullUAVs.data(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, m_pHiZBufferTarget->GetUAV(mip - 1u).GetAddressOf(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, m_pHiZBufferTarget->GetUAV(mip).GetAddressOf(), nullptr);

				m_pHiZCreateMipKernel->Dispatch(gfx, dstWidth, dstHeight, 1u);
			}

			pass.UnbindSharedResources(gfx);
		}

		// Normal-rough-reflectivity pass
		{
			const RenderPass& pass = GetRenderPass(GBufferRenderPassName);

			pass.BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);
			m_pNormalRoughReflectivityTarget->ClearRenderTarget(context.Get(), 1.f, 0.f, 0.f, 1.f);

			gfx.SetRenderTarget(m_pNormalRoughReflectivityTarget->GetRenderTargetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass.Execute(gfx);

			gfx.ClearRenderTargets();

			pass.UnbindSharedResources(gfx);
		}

		// Tiled lighting pass
		{
			const RenderPass& pass = GetRenderPass(TiledLightingPassName);
			pass.BindSharedResources(gfx);

			m_pTiledLightingKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass.UnbindSharedResources(gfx);
		}

		// Clustered lighting pass
		{
			const RenderPass& pass = GetRenderPass(ClusteredLightingPassName);
			pass.BindSharedResources(gfx);

			static ClusteredLightingCB clusteredLightingCB;
			clusteredLightingCB.groupResolutions = { m_pLightManager->GetClusterDimensionX(), m_pLightManager->GetClusterDimensionY(), m_pLightManager->GetClusterDimensionZ(), 0u };
			m_pClusteredLightingCB->Update(gfx, clusteredLightingCB);

			m_pClusteredLightingKernel->Dispatch(gfx, m_pLightManager->GetClusterDimensionX(), m_pLightManager->GetClusterDimensionY(), m_pLightManager->GetClusterDimensionZ());

			pass.UnbindSharedResources(gfx);
		}

		// Opaque pass
		{
			const RenderPass& pass = GetRenderPass(OpaqueRenderPassName);
			pass.BindSharedResources(gfx);

			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Gbuffer)->BindOM(gfx);

			m_pCameraColor0->ClearRenderTarget(context.Get(), 0.f, 0.f, 0.f, 0.f);
			m_pCameraColor0->BindAsTarget(gfx, gfx.GetDepthStencilTarget()->GetView());
			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());

			pass.Execute(gfx);
			pass.UnbindSharedResources(gfx);

			gfx.ClearRenderTargets();
		}

		// Blur pyramid
		{
			/*const RenderPass& pass = GetRenderPass(BlurPyramidPassName);
			pass.BindSharedResources(gfx);

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
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, pass.pNullUAVs.data(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, pHiZBufferTarget->GetUAV(mip - 1u).GetAddressOf(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, pHiZBufferTarget->GetUAV(mip).GetAddressOf(), nullptr);

				pHiZCreateMipKernel->Dispatch(gfx, dstWidth, dstHeight, 1u);
			}

			pass.UnbindSharedResources(gfx);*/
		}

		// SSR pass
		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			const RenderPass& pass = GetRenderPass(SSRRenderPassName);
			pass.BindSharedResources(gfx);

			static SSR_CB ssrCB;
			ssrCB.debugViewStep = (frameCt / 20u) % 25u;
			m_pSSR_CB->Update(gfx, ssrCB);

			m_pSSRKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass.UnbindSharedResources(gfx);
		}

		// FXAA pass
		if (Config::AAType == Config::AAType::FXAA && IsFeatureEnabled(RendererFeature::FXAA))
		{
			const RenderPass& pass = GetRenderPass(FXAARenderPassName);
			pass.BindSharedResources(gfx);

			static FXAA_CB fxaaCB;
			fxaaCB.minThreshold = 0.1f;
			fxaaCB.maxThreshold = 2.f;
			fxaaCB.edgeSharpness = 0.25f;
			fxaaCB.padding = 0.f;
			m_pFXAA_CB->Update(gfx, fxaaCB);

			m_pFXAAKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass.UnbindSharedResources(gfx);
		}

		// Dither pass
		if (m_viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Dither))
		{
			const RenderPass& pass = GetRenderPass(DitherRenderPassName);
			pass.BindSharedResources(gfx);

			static DitherCB ditherCB;
			ditherCB.shadowDither = 0.15f;
			ditherCB.midDither = 0.04f;
			m_pDitherCB->Update(gfx, ditherCB);

			m_pDitherKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass.UnbindSharedResources(gfx);
		}

		// Tonemapping pass
		if (m_viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Tonemapping))
		{
			const RenderPass& pass = GetRenderPass(TonemappingRenderPassName);
			pass.BindSharedResources(gfx);

			m_pTonemappingKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1);

			pass.UnbindSharedResources(gfx);
		}

		// Final blit
		{
			RenderPass& pass = GetRenderPass(FinalBlitRenderPassName);
			auto& fsPass = static_cast<FullscreenPass&>(pass);

			// todo: remove this - for now it's needed to clear SRVs
			context->ClearState();

			pass.BindSharedResources(gfx);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx);

			// Debug view overrides: (do this here so it can be changed dynamically later)
			switch (m_viewIdx)
			{
			case RendererView::Final:
				fsPass.SetInputTarget(m_pFinalBlitInputIsIndex0 ? m_pCameraColor0 : m_pCameraColor1);
				break;
			case RendererView::TiledLighting:
				fsPass.SetInputTarget(m_pDebugTiledLighting);
				break;
			case RendererView::ClusteredLighting:
				fsPass.SetInputTarget(m_pDebugClusteredLighting);
				break;
			case RendererView::SSRTrace:
				fsPass.SetInputTarget(m_pDebugSSR);
				break;
			}

			gfx.SetViewport(gfx.GetScreenWidth(), gfx.GetScreenHeight());
			gfx.SetRenderTarget(gfx.GetBackBufferView());

			pass.Execute(gfx);
			pass.UnbindSharedResources(gfx);
		}

		gfx.GetRenderStats().EndFrame();

	}

	void Renderer::DrawImguiControlWindow(const GraphicsDevice& gfx)
	{
		if (ImGui::Begin("Renderer"))
		{
			static ImVec2 buttonSize = { 110, 25 };
			static ImVec2 featureButtonSize = { 70, 18 };

			bool changed = false;

			// Select view
			ImGui::Text("View:");
			int newViewIdx = (int)m_viewIdx;
			newViewIdx = DrawSelectableButtonInArray(0, "Final", newViewIdx, buttonSize, changed, false);
			newViewIdx = DrawSelectableButtonInArray(1, "Tiled Lighting", newViewIdx, buttonSize, changed, true);
			newViewIdx = DrawSelectableButtonInArray(2, "Clustered Lighting", newViewIdx, buttonSize, changed, true);
			newViewIdx = DrawSelectableButtonInArray(3, "SSR Trace", newViewIdx, buttonSize, changed, true);
			m_viewIdx = (RendererView)newViewIdx;

			int _pixelIteration = m_pixelIteration;
			ImGui::SliderInt("Iteration:", &_pixelIteration, 0, 100);
			m_pixelIteration = _pixelIteration;

			// Toggle features
			ImGui::Text("Features:");

			//static ImGuiTableFlags flags = ImGuiTableFlags_PreciseWidths;
			ImGui::BeginTable("FeatureTable", 2);
			m_rendererFeatureEnabled[(int)RendererFeature::Shadows] = DrawToggleOnOffButton(0, "Shadows", m_rendererFeatureEnabled[(int)RendererFeature::Shadows], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::FXAA] = DrawToggleOnOffButton(1, "FXAA", m_rendererFeatureEnabled[(int)RendererFeature::FXAA], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::HZBSSR] = DrawToggleOnOffButton(2, "Hi-Z SSR", m_rendererFeatureEnabled[(int)RendererFeature::HZBSSR], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::Dither] = DrawToggleOnOffButton(3, "Dither", m_rendererFeatureEnabled[(int)RendererFeature::Dither], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::Tonemapping] = DrawToggleOnOffButton(4, "Tonemapping", m_rendererFeatureEnabled[(int)RendererFeature::Tonemapping], featureButtonSize, changed);
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
		for (auto& p : m_pRenderPasses)
		{
			p.second->Reset();
		}
	}

	RenderPass& Renderer::GetRenderPass(const std::string name) const
	{
		return *m_pRenderPasses.at(name).get();
	}

	const RenderPass& Renderer::CreateRenderPass(const std::string name)
	{
		m_pRenderPasses.emplace(name, std::make_unique<RenderPass>(name));
		return *m_pRenderPasses[name].get();
	}

	const RenderPass& Renderer::CreateRenderPass(const std::string name, std::unique_ptr<RenderPass> pRenderPass)
	{
		m_pRenderPasses.emplace(name, std::move(pRenderPass));
		return *m_pRenderPasses[name].get();
	}

	bool Renderer::IsFeatureEnabled(RendererFeature feature) const
	{
		return m_rendererFeatureEnabled[(int)feature];
	}
}