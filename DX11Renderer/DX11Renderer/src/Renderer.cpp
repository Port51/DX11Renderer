#include "pch.h"
#include <array>
#include <functional>
#include "Renderer.h"
#include "DepthStencilState.h"
#include "GraphicsDevice.h"
#include "DrawCommand.h"
#include "RenderPass.h"
#include "FullscreenPass.h"
#include "SkyboxPass.h"
#include "DepthOfFieldPass.h"
#include "NullPixelShader.h"
#include "RenderTexture.h"
#include "DepthStencilTarget.h"
#include "ComputeShader.h"
#include "ComputeKernel.h"
#include "StructuredBuffer.h"
#include "ConstantBuffer.h"
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
#include "RenderConstants.h"
#include "BloomPass.h"
#include "SSAOPass.h"
#include "RandomGenerator.h"
#include "ParticleManager.h"
#include <thread>

namespace gfx
{
	Renderer::Renderer(const GraphicsDevice& gfx, RandomGenerator& rng, std::shared_ptr<LightManager> pLightManager, std::shared_ptr<ParticleManager> pParticleManager, std::shared_ptr<RendererList> pRendererList)
		: m_pRendererList(std::move(pRendererList)), m_pLightManager(std::move(pLightManager)), m_pParticleManager(std::move(pParticleManager))
	{
		UINT screenWidth = gfx.GetScreenWidth();
		UINT screenHeight = gfx.GetScreenHeight();

		//
		// Debug stuff
		//
		m_rendererFeatureEnabled.resize(RendererFeature::COUNT, true); // enable all features by default
		m_rendererFeatureEnabled[RendererFeature::HZBSSR] = true;
		m_rendererFeatureEnabled[RendererFeature::DepthOfField] = true;
		m_rendererFeatureEnabled[RendererFeature::Bloom] = true;

		//
		// Components
		//
		m_pVisibleRendererList = std::make_unique<RendererList>(m_pRendererList);
		m_pVisibleTransparentRendererList = std::make_unique<RendererList>(m_pRendererList);

		//
		// Texture assets and samplers
		//
		m_pDitherTexture = std::dynamic_pointer_cast<Texture>(Texture::Resolve(gfx, "Assets\\Textures\\Dither8x8.png"));
		m_pRGBNoiseTexture = std::dynamic_pointer_cast<Texture>(Texture::Resolve(gfx, "Assets\\Textures\\RGBNoise32x32.png"));

		m_pPointWrapSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP);
		m_pPointClampSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);
		m_pPointMirrorSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR);
		m_pBilinearWrapSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP);
		m_pBilinearClampSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);
		m_pBilinearMirrorSampler = std::make_shared<Sampler>(gfx, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR);
		
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
		m_pNormalRoughReflectivityTarget->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pHiZBufferTarget = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16_UNORM, 8u);
		m_pHiZBufferTarget->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pSpecularLighting = std::make_shared<RenderTexture>(gfx);
		m_pSpecularLighting->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pDiffuseLighting = std::make_shared<RenderTexture>(gfx);
		m_pDiffuseLighting->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pCameraColor0 = std::make_shared<RenderTexture>(gfx);
		m_pCameraColor0->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pCameraColor1 = std::make_shared<RenderTexture>(gfx);
		m_pCameraColor1->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pDownsampledColor = std::make_shared<RenderTexture>(gfx);
		m_pDownsampledColor->Init(gfx.GetAdapter(), screenWidth >> 1u, screenHeight >> 1u);

		m_pDebugTiledLighting = std::make_shared<RenderTexture>(gfx);
		m_pDebugTiledLighting->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pDebugClusteredLighting = std::make_shared<RenderTexture>(gfx);
		m_pDebugClusteredLighting->Init(gfx.GetAdapter(), screenWidth, screenHeight);

		m_pDebugSSR = std::make_shared<RenderTexture>(gfx);
		m_pDebugSSR->Init(gfx.GetAdapter(), screenWidth, screenHeight);

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
		m_pHiZDepthCopyKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "HiZDepthCopy.cso"));
		m_pHiZCreateMipKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "HiZCreateMip.cso"));
		m_pTiledLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "TiledLightingCompute.cso"));
		m_pClusteredLightingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "ClusteredLightingCompute.cso"));
		m_pBilinearDownsampleKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "BilinearDownsample.cso"));
		m_pSSRKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "SSR.cso"));
		m_pFXAAKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "FXAA.cso"));
		m_pDitherKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Dither.cso"));
		m_pTonemappingKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Tonemapping.cso"));

		//
		// Render passes
		//
		CreateRenderPass(RenderPassType::PerCameraRenderPass);
		CreateRenderPass(RenderPassType::DepthPrepassRenderPass);
		CreateRenderPass(RenderPassType::HiZRenderPass);
		CreateRenderPass(RenderPassType::ShadowRenderPass);
		CreateRenderPass(RenderPassType::GBufferRenderPass);
		CreateRenderPass(RenderPassType::TiledLightingRenderPass);
		CreateRenderPass(RenderPassType::ClusteredLightingRenderPass);
		CreateRenderPass(RenderPassType::OpaqueRenderPass);
		CreateRenderPass(RenderPassType::SkyboxRenderPass, std::move(std::make_unique<SkyboxPass>(gfx)));
		CreateRenderPass(RenderPassType::TransparentRenderPass);
		CreateRenderPass(RenderPassType::CreateDownsampledX2Texture);
		CreateRenderPass(RenderPassType::DepthOfFieldRenderPass, std::move(std::make_unique<DepthOfFieldPass>(gfx, DepthOfFieldPass::DepthOfFieldBokehType::DiskBokeh)));
		CreateRenderPass(RenderPassType::BloomRenderPass, std::move(std::make_unique<BloomPass>(gfx)));
		CreateRenderPass(RenderPassType::SSAORenderPass, std::move(std::make_unique<SSAOPass>(gfx, rng)));
		CreateRenderPass(RenderPassType::SSRRenderPass);
		CreateRenderPass(RenderPassType::FXAARenderPass);
		CreateRenderPass(RenderPassType::DitherRenderPass);
		CreateRenderPass(RenderPassType::TonemappingRenderPass);

		CreateRenderPass(RenderPassType::FinalBlitRenderPass,
			std::move(std::make_unique<FullscreenPass>(gfx, RenderPassType::FinalBlitRenderPass, "BlitPS.cso")));

		SetupRenderPassDependencies(gfx);
	}

	Renderer::~Renderer()
	{

	}

	void Renderer::Release()
	{
		m_pLightManager->Release();

		m_pHiZBufferTarget->Release();
		m_pCameraColor0->Release();
		m_pCameraColor1->Release();
		m_pNormalRoughReflectivityTarget->Release();
		m_pDiffuseLighting->Release();
		m_pSpecularLighting->Release();
		m_pDownsampledColor->Release();
		m_pDitherTexture->Release();

		m_pPointWrapSampler->Release();
		m_pPointClampSampler->Release();
		m_pBilinearWrapSampler->Release();
		m_pBilinearClampSampler->Release();
		m_pShadowSampler->Release();

		m_pClusteredLightingCB->Release();
		m_pDitherCB->Release();
		m_pFXAA_CB->Release();
		m_pHiZCreationCB->Release();
		m_pPerCameraCB->Release();
		m_pPerFrameCB->Release();
		m_pSSR_CB->Release();
		m_pTransformationCB->Release();

		m_pDebugClusteredLighting->Release();
		m_pDebugSSR->Release();
		m_pDebugTiledLighting->Release();
		m_pSSR_DebugData->Release();
	}

	void Renderer::SetupRenderPassDependencies(const GraphicsDevice& gfx)
	{
		bool cameraOutSlot0 = true;

		GetRenderPass(RenderPassType::PerCameraRenderPass).
			ClearBinds()
			.CSSetCB(RenderSlots::CS_PerFrameCB, m_pPerFrameCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_GlobalTransformsCB, m_pTransformationCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_PerCameraCB, m_pPerCameraCB->GetD3DBuffer())
			.CSSetCB(RenderSlots::CS_LightInputCB, m_pLightManager->GetLightInputCB().GetD3DBuffer())
			.CSSetSRV(RenderSlots::CS_LightDataSRV, m_pLightManager->GetLightDataSRV())
			.CSSetSRV(RenderSlots::CS_LightShadowDataSRV, m_pLightManager->GetShadowDataSRV())
			.VSSetCB(RenderSlots::VS_PerFrameCB, m_pPerFrameCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_GlobalTransformsCB, m_pTransformationCB->GetD3DBuffer())
			.VSSetCB(RenderSlots::VS_PerCameraCB, m_pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerFrameCB, m_pPerFrameCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_GlobalTransformsCB, m_pTransformationCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_PerCameraCB, m_pPerCameraCB->GetD3DBuffer())
			.PSSetCB(RenderSlots::PS_LightInputCB, m_pLightManager->GetLightInputCB().GetD3DBuffer())
			.CSSetSPL(RenderSlots::Global_PointWrapSampler, m_pPointWrapSampler->GetD3DSampler())
			.CSSetSPL(RenderSlots::Global_PointClampSampler, m_pPointClampSampler->GetD3DSampler())
			.CSSetSPL(RenderSlots::Global_PointMirrorSampler, m_pPointMirrorSampler->GetD3DSampler())
			.CSSetSPL(RenderSlots::Global_BilinearWrapSampler, m_pBilinearWrapSampler->GetD3DSampler())
			.CSSetSPL(RenderSlots::Global_BilinearClampSampler, m_pBilinearClampSampler->GetD3DSampler())
			.CSSetSPL(RenderSlots::Global_BilinearMirrorSampler, m_pBilinearMirrorSampler->GetD3DSampler())
			.VSSetSPL(RenderSlots::Global_PointWrapSampler, m_pPointWrapSampler->GetD3DSampler())
			.VSSetSPL(RenderSlots::Global_PointClampSampler, m_pPointClampSampler->GetD3DSampler())
			.VSSetSPL(RenderSlots::Global_PointMirrorSampler, m_pPointMirrorSampler->GetD3DSampler())
			.VSSetSPL(RenderSlots::Global_BilinearWrapSampler, m_pBilinearWrapSampler->GetD3DSampler())
			.VSSetSPL(RenderSlots::Global_BilinearClampSampler, m_pBilinearClampSampler->GetD3DSampler())
			.VSSetSPL(RenderSlots::Global_BilinearMirrorSampler, m_pBilinearMirrorSampler->GetD3DSampler())
			.PSSetSPL(RenderSlots::Global_PointWrapSampler, m_pPointWrapSampler->GetD3DSampler())
			.PSSetSPL(RenderSlots::Global_PointClampSampler, m_pPointClampSampler->GetD3DSampler())
			.PSSetSPL(RenderSlots::Global_PointMirrorSampler, m_pPointMirrorSampler->GetD3DSampler())
			.PSSetSPL(RenderSlots::Global_BilinearWrapSampler, m_pBilinearWrapSampler->GetD3DSampler())
			.PSSetSPL(RenderSlots::Global_BilinearClampSampler, m_pBilinearClampSampler->GetD3DSampler())
			.PSSetSPL(RenderSlots::Global_BilinearMirrorSampler, m_pBilinearMirrorSampler->GetD3DSampler());

		GetRenderPass(RenderPassType::DepthPrepassRenderPass).
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(RenderPassType::HiZRenderPass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pHiZBufferTarget->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, nullptr)
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pHiZCreationCB->GetD3DBuffer())
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(RenderPassType::ShadowRenderPass).
			ClearBinds()
			.VSSetCB(RenderSlots::VS_GlobalTransformsCB, m_pTransformationCB->GetD3DBuffer())
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK)).SetupRSBinding();
			//.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_MODE::D3D11_CULL_FRONT)).SetupRSBinding(); // Reduce shadow acne w/ front face culling during shadow pass

		GetRenderPass(RenderPassType::GBufferRenderPass).
			ClearBinds()
			.AddBinding(RasterizerState::Resolve(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK)).SetupRSBinding();

		GetRenderPass(RenderPassType::TiledLightingRenderPass).
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

		if (IsFeatureEnabled(RendererFeature::SSAO))
		{
			GetRenderPass(RenderPassType::TiledLightingRenderPass)
				.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, static_cast<SSAOPass&>(GetRenderPass(RenderPassType::SSAORenderPass)).GetOcclusionTexture().GetSRV());
		}

		GetRenderPass(RenderPassType::ClusteredLightingRenderPass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, gfx.GetDepthStencilTarget()->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pHiZBufferTarget->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pLightManager->GetClusteredIndices().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDebugClusteredLighting->GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pClusteredLightingCB->GetD3DBuffer());

		GetRenderPass(RenderPassType::OpaqueRenderPass).
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

		static_cast<SkyboxPass&>(GetRenderPass(RenderPassType::SkyboxRenderPass)).SetupRenderPassDependencies(gfx, gfx.GetDepthStencilTarget()->GetSRV(), *m_pCameraColor0);

		GetRenderPass(RenderPassType::TransparentRenderPass).
			ClearBinds()
			.PSSetSRV(RenderSlots::PS_FreeSRV + 2u, m_pLightManager->GetClusteredIndices().GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 3u, m_pDitherTexture->GetSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 4u, m_pLightManager->GetLightDataSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 5u, m_pLightManager->GetShadowDataSRV())
			.PSSetSRV(RenderSlots::PS_FreeSRV + 6u, m_pLightManager->GetShadowAtlas().GetSRV())
			.PSSetCB(RenderSlots::PS_FreeCB + 1u, m_pClusteredLightingCB->GetD3DBuffer())
			.PSSetSPL(RenderSlots::PS_FreeSPL + 0u, m_pShadowSampler->GetD3DSampler());

		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(RenderPassType::SSRRenderPass).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_GbufferNormalRoughSRV, m_pNormalRoughReflectivityTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, gfx.GetDepthStencilTarget()->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, m_pHiZBufferTarget->GetSRV())
				.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, m_pDitherTexture->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pSSR_DebugData->GetUAV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, m_pDebugSSR->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pSSR_CB->GetD3DBuffer());
		}

		// Downsampled FX
		{
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			//cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(RenderPassType::CreateDownsampledX2Texture).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDownsampledColor->GetUAV());

			static_cast<DepthOfFieldPass&>(GetRenderPass(RenderPassType::DepthOfFieldRenderPass)).SetupRenderPassDependencies(gfx, *m_pDownsampledColor, *m_pHiZBufferTarget, *pColorIn);

			static_cast<BloomPass&>(GetRenderPass(RenderPassType::BloomRenderPass)).SetupRenderPassDependencies(gfx, *m_pDownsampledColor, *pColorIn);
		}

		if (IsFeatureEnabled(RendererFeature::SSAO))
		{
			static_cast<SSAOPass&>(GetRenderPass(RenderPassType::SSAORenderPass)).SetupRenderPassDependencies(gfx, *m_pNormalRoughReflectivityTarget, *m_pHiZBufferTarget, *m_pRGBNoiseTexture);
		}
		
		if (IsFeatureEnabled(RendererFeature::FXAA))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(RenderPassType::FXAARenderPass).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV())
				.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pFXAA_CB->GetD3DBuffer());
		}

		if (IsFeatureEnabled(RendererFeature::Dither))
		{
			// Assign inputs and outputs
			const auto& pColorIn = (cameraOutSlot0) ? m_pCameraColor0 : m_pCameraColor1;
			const auto& pColorOut = (cameraOutSlot0) ? m_pCameraColor1 : m_pCameraColor0;
			cameraOutSlot0 = !cameraOutSlot0;

			GetRenderPass(RenderPassType::DitherRenderPass).
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

			GetRenderPass(RenderPassType::TonemappingRenderPass).
				ClearBinds()
				.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pColorIn->GetSRV())
				.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pColorOut->GetUAV());
		}

		// Determine what to use for final blit
		m_pFinalBlitInputIsIndex0 = cameraOutSlot0;
	}

	void Renderer::AcceptDrawCall(GraphicsDevice& gfx, DrawCommand job, const RenderPassType targetPass)
	{
		gfx.GetRenderStats().RegisterCPUDrawCall();
		m_pRenderPasses[targetPass]->EnqueueJob(std::move(job));
	}

	void Renderer::Execute(GraphicsDevice& gfx, const Camera& camera, const float timeStep, const float timeElapsed, const UINT pixelSelectionX, const UINT pixelSelectionY)
	{
		auto context = gfx.GetContext();
		const UINT screenWidth = gfx.GetScreenWidth();
		const UINT screenHeight = gfx.GetScreenHeight();
		const UINT halfScreenWidth = gfx.GetScreenWidth() >> 1u;
		const UINT halfScreenHeight = gfx.GetScreenHeight() >> 1u;

		gfx.GetRenderStats().StartFrame();
		context->ClearState();
		RenderState renderState;

		static int frameCt = 0;
		frameCt++;

		// Shadow + lighting culling pass
		{
			RenderPass& pass = GetRenderPass(RenderPassType::ShadowRenderPass);

			pass.BindSharedResources(gfx, renderState);

			static ShadowPassContext context(gfx, camera, *this, pass, *m_pTransformationCB.get(), nullptr);

			m_pLightManager->CullLightsAndShadows(gfx, camera, IsFeatureEnabled(RendererFeature::Shadows)); // changes the SRV, which will be bound in per-frame binds
			if (IsFeatureEnabled(RendererFeature::Shadows))
			{
				m_pLightManager->RenderShadows(context, renderState);
			}

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Submit draw call threads
		std::vector<std::thread> filterThreads;
		std::vector<std::thread> drawCallThreads;
		{
			static DrawContext opaqueDrawContext(*this, std::move(std::vector<RenderPassType> { RenderPassType::DepthPrepassRenderPass, RenderPassType::GBufferRenderPass, RenderPassType::OpaqueRenderPass }));
			opaqueDrawContext.viewMatrix = camera.GetViewMatrix();
			opaqueDrawContext.projMatrix = camera.GetProjectionMatrix();

			static DrawContext transparentDrawContext(*this, std::move(std::vector<RenderPassType> { RenderPassType::TransparentRenderPass }));
			transparentDrawContext.viewMatrix = camera.GetViewMatrix();
			transparentDrawContext.projMatrix = camera.GetProjectionMatrix();

			//m_pVisibleRendererList->Filter(gfx, camera.GetFrustumWS(), RendererList::RendererSortingType::StateThenBackToFront, RenderPassType::OpaqueRenderPass, camera.GetPositionWS(), camera.GetForwardWS(), camera.GetFarClipPlane());
			//m_pVisibleTransparentRendererList->Filter(gfx, camera.GetFrustumWS(), RendererList::RendererSortingType::BackToFrontThenState, RenderPassType::TransparentRenderPass, camera.GetPositionWS(), camera.GetForwardWS(), camera.GetFarClipPlane());
			filterThreads.push_back(std::thread(&RendererList::Filter, m_pVisibleRendererList.get(), std::ref(gfx), std::ref(camera.GetFrustumWS()), RendererList::RendererSortingType::StateThenBackToFront, RenderPassType::OpaqueRenderPass, camera.GetPositionWS(), camera.GetForwardWS(), camera.GetFarClipPlane()));
			filterThreads.push_back(std::thread(&RendererList::Filter, m_pVisibleTransparentRendererList.get(), std::ref(gfx), std::ref(camera.GetFrustumWS()), RendererList::RendererSortingType::BackToFrontThenState, RenderPassType::TransparentRenderPass, camera.GetPositionWS(), camera.GetForwardWS(), camera.GetFarClipPlane()));

			for (auto& t : filterThreads)
			{
				if (t.joinable()) t.join();
			}

			//m_pVisibleRendererList->SubmitDrawCalls(gfx, opaqueDrawContext);
			//m_pVisibleTransparentRendererList->SubmitDrawCalls(gfx, transparentDrawContext);
			drawCallThreads.push_back(std::thread(&RendererList::SubmitDrawCalls, m_pVisibleRendererList.get(), std::ref(gfx), std::ref(opaqueDrawContext)));
			drawCallThreads.push_back(std::thread(&RendererList::SubmitDrawCalls, m_pVisibleTransparentRendererList.get(), std::ref(gfx), std::ref(transparentDrawContext)));
		}

		// Early frame calculations
		{
			static PerFrameCB perFrameCB;
			ZERO_MEM(perFrameCB);
			perFrameCB.pixelSelection = { pixelSelectionX, pixelSelectionY, (UINT)m_pixelIteration, 0u };
			perFrameCB.time = dx::XMVectorSet(timeElapsed / 20.f, timeElapsed, timeElapsed * 2.f, timeElapsed * 3.f);
			perFrameCB.sinTime = dx::XMVectorSet(std::sin(timeElapsed / 8.f), std::sin(timeElapsed / 4.f), std::sin(timeElapsed / 2.f), std::sin(timeElapsed));
			perFrameCB.cosTime = dx::XMVectorSet(std::cos(timeElapsed / 8.f), std::cos(timeElapsed / 4.f), std::cos(timeElapsed / 2.f), std::cos(timeElapsed));
			perFrameCB.timeStep = dx::XMVectorSet(timeStep, std::min(timeStep, 1.f / 30.f), std::min(timeStep, 1.f / 60.f), timeStep);
			m_pPerFrameCB->Update(gfx, perFrameCB);

			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = camera.GetViewMatrix();
			transformationCB.projMatrix = camera.GetProjectionMatrix();
			transformationCB.viewProjMatrix = transformationCB.viewMatrix * transformationCB.projMatrix;
			transformationCB.invViewMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewMatrix);
			transformationCB.invProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.projMatrix);
			transformationCB.invViewProjMatrix = dx::XMMatrixInverse(nullptr, transformationCB.viewProjMatrix);
			m_pTransformationCB->Update(gfx, transformationCB);

			const float farNearRatio = camera.GetFarClipPlane() / camera.GetNearClipPlane();

			static PerCameraCB perCameraCB;
			ZERO_MEM(perCameraCB);
			perCameraCB.projectionParams = dx::XMVectorSet(1.f, camera.GetNearClipPlane(), camera.GetFarClipPlane(), 1.f / camera.GetFarClipPlane());
			perCameraCB.screenParams = dx::XMVectorSet(screenWidth, screenHeight, 1.f / screenWidth, 1.f / screenHeight);
			perCameraCB.zBufferParams = dx::XMVectorSet(1.f - farNearRatio, farNearRatio, 1.f / camera.GetFarClipPlane() - 1.f / camera.GetNearClipPlane(), 1.f / camera.GetNearClipPlane());
			perCameraCB.orthoParams = dx::XMVectorSet(0.f, 0.f, 0.f, 0.f);
			perCameraCB.frustumCornerDataVS = camera.GetFrustumCornersVS();
			perCameraCB.inverseFrustumCornerDataVS = camera.GetInverseFrustumCornersVS();
			perCameraCB.frustumPlaneDirVS = camera.GetGPUFrustumPlaneDirVS();
			perCameraCB.cameraPositionWS = camera.GetPositionWS();
			perCameraCB.useOcclusion = IsFeatureEnabled(RendererFeature::SSAO) ? 1u : 0u;

			// todo: move elsewhere, and only calculate when FOV or resolution changes?
			float fClustersZ = (float)m_pLightManager->GetClusterDimensionZ();
			float logFarOverNear = std::log2f(farNearRatio);
			perCameraCB.clusterPrecalc = dx::XMVectorSet(fClustersZ / logFarOverNear, -(fClustersZ * std::log2f(camera.GetNearClipPlane()) / logFarOverNear), 0.f, 0.f);

			// This is used when calculating cluster.xy from NDC
			// These calculations turn it into a single [MAD] operation
			const float invClusterDim = 1.f / 16.f;
			perCameraCB.clusterXYRemap = dx::XMVectorSet(
				screenWidth * 0.5f * invClusterDim,
				screenHeight * 0.5f * invClusterDim,
				0.f,
				0.f
			);
			m_pPerCameraCB->Update(gfx, perCameraCB);
		}

		// Per-frame and per-camera binds
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::PerCameraRenderPass);
			pass.BindSharedResources(gfx, renderState);
		}

		// Wait for draw call threads to complete
		{
			for (auto& t : drawCallThreads)
			{
				if (t.joinable()) t.join();
			}
		}

		// Early Z pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::DepthPrepassRenderPass);

			pass.BindSharedResources(gfx, renderState);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx, renderState);
			gfx.GetDepthStencilTarget()->Clear(gfx);

			gfx.SetDepthOnlyRenderTarget();
			gfx.SetViewport(screenWidth, screenHeight);

			pass.Execute(gfx, renderState);
			pass.UnbindSharedResources(gfx, renderState);
		}

		// Hi-Z buffer pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::HiZRenderPass);

			gfx.ClearRenderTargets(); // need in order to access depth
			pass.BindSharedResources(gfx, renderState);

			static HiZCreationCB hiZCreationCB;
			hiZCreationCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			m_pHiZCreationCB->Update(gfx, hiZCreationCB);

			// Copy from depth-stencil
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV, 1u, m_pHiZBufferTarget->GetUAV(0u).GetAddressOf(), nullptr);
			m_pHiZDepthCopyKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

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
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, RenderConstants::NullUAVArray.data(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, m_pHiZBufferTarget->GetUAV(mip - 1u).GetAddressOf(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, m_pHiZBufferTarget->GetUAV(mip).GetAddressOf(), nullptr);

				m_pHiZCreateMipKernel->Dispatch(gfx, dstWidth, dstHeight, 1u);
			}

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Run GPU particle compute
		{
			//m_pParticleManager->ExecuteComputePass(gfx, camera, renderState);
		}

		// Normal-rough-reflectivity pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::GBufferRenderPass);

			pass.BindSharedResources(gfx, renderState);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Normal)->BindOM(gfx, renderState);
			m_pNormalRoughReflectivityTarget->ClearRenderTarget(context.Get(), 1.f, 0.f, 0.f, 1.f);

			gfx.SetRenderTarget(m_pNormalRoughReflectivityTarget->GetRenderTargetView());
			gfx.SetViewport(screenWidth, screenHeight);

			pass.Execute(gfx, renderState);

			gfx.ClearRenderTargets();

			pass.UnbindSharedResources(gfx, renderState);
		}

		if (IsFeatureEnabled(RendererFeature::SSAO))
		{
			GetRenderPass(SSAORenderPass).Execute(gfx, renderState);
		}

		// Tiled lighting pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::TiledLightingRenderPass);
			pass.BindSharedResources(gfx, renderState);

			m_pTiledLightingKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Clustered lighting pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::ClusteredLightingRenderPass);
			pass.BindSharedResources(gfx, renderState);

			static ClusteredLightingCB clusteredLightingCB;
			clusteredLightingCB.groupResolutions = { m_pLightManager->GetClusterDimensionX(), m_pLightManager->GetClusterDimensionY(), m_pLightManager->GetClusterDimensionZ(), 0u };
			m_pClusteredLightingCB->Update(gfx, clusteredLightingCB);

			m_pClusteredLightingKernel->Dispatch(gfx, m_pLightManager->GetClusterDimensionX(), m_pLightManager->GetClusterDimensionY(), m_pLightManager->GetClusterDimensionZ());

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Opaque pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::OpaqueRenderPass);
			pass.BindSharedResources(gfx, renderState);

			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Normal)->BindOM(gfx, renderState);

			m_pCameraColor0->ClearRenderTarget(context.Get(), 0.f, 0.f, 0.f, 0.f);
			m_pCameraColor0->BindAsTarget(gfx, gfx.GetDepthStencilTarget()->GetView());
			gfx.SetViewport(screenWidth, screenHeight);

			pass.Execute(gfx, renderState);
			pass.UnbindSharedResources(gfx, renderState);

			gfx.ClearRenderTargets();
		}

		// Skybox pass
		{
			GetRenderPass(SkyboxRenderPass).Execute(gfx, renderState);
		}

		// Transparent pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::TransparentRenderPass);
			pass.BindSharedResources(gfx, renderState);

			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::Normal)->BindOM(gfx, renderState);

			m_pCameraColor0->BindAsTarget(gfx, gfx.GetDepthStencilTarget()->GetView());
			gfx.SetViewport(screenWidth, screenHeight);

			pass.Execute(gfx, renderState);
			pass.UnbindSharedResources(gfx, renderState);

			gfx.ClearRenderTargets();
		}

		// SSR pass
		if (IsFeatureEnabled(RendererFeature::HZBSSR))
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::SSRRenderPass);
			pass.BindSharedResources(gfx, renderState);

			static SSR_CB ssrCB;
			ssrCB.debugViewStep = (frameCt / 20u) % 25u;
			m_pSSR_CB->Update(gfx, ssrCB);

			m_pSSRKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Downsample x2 pass
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::CreateDownsampledX2Texture);
			pass.BindSharedResources(gfx, renderState);

			m_pBilinearDownsampleKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);

			pass.Execute(gfx, renderState);
			pass.UnbindSharedResources(gfx, renderState);
		}

		if (m_viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::DepthOfField))
		{
			GetRenderPass(DepthOfFieldRenderPass).Execute(gfx, renderState);
		}

		if (m_viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Bloom))
		{
			GetRenderPass(BloomRenderPass).Execute(gfx, renderState);
		}

		// Run GPU particle render pass
		{
			//m_pParticleManager->ExecuteRenderPass(gfx, camera, renderState);
		}

		// FXAA pass
		if (Config::AAType == Config::AAType::FXAA && IsFeatureEnabled(RendererFeature::FXAA))
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::FXAARenderPass);
			pass.BindSharedResources(gfx, renderState);

			static FXAA_CB fxaaCB;
			fxaaCB.minThreshold = 0.1f;
			fxaaCB.maxThreshold = 2.f;
			fxaaCB.edgeSharpness = 0.25f;
			fxaaCB.padding = 0.f;
			m_pFXAA_CB->Update(gfx, fxaaCB);

			m_pFXAAKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Dither pass
		if (m_viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Dither))
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::DitherRenderPass);
			pass.BindSharedResources(gfx, renderState);

			static DitherCB ditherCB;
			ditherCB.shadowDither = 0.15f;
			ditherCB.midDither = 0.04f;
			m_pDitherCB->Update(gfx, ditherCB);

			m_pDitherKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Tonemapping pass
		if (m_viewIdx == RendererView::Final && IsFeatureEnabled(RendererFeature::Tonemapping))
		{
			const RenderPass& pass = GetRenderPass(RenderPassType::TonemappingRenderPass);
			pass.BindSharedResources(gfx, renderState);

			m_pTonemappingKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Final blit
		{
			RenderPass& pass = GetRenderPass(RenderPassType::FinalBlitRenderPass);
			auto& fsPass = static_cast<FullscreenPass&>(pass);

			pass.BindSharedResources(gfx, renderState);
			DepthStencilState::Resolve(gfx, DepthStencilState::Mode::StencilOff)->BindOM(gfx, renderState);

			// Debug view overrides: (do this here so it can be changed dynamically later)
			switch (m_viewIdx)
			{
			case RendererView::Final:
				//fsPass.SetInputTarget(m_pBloomTarget0);
				//fsPass.SetInputTarget(m_pDownsampledColor);
				//fsPass.SetInputTarget(m_pDoFFar3);
				//fsPass.SetInputTarget(m_pDoFNear0);
				//fsPass.SetInputTarget(static_cast<SSAOPass&>(GetRenderPass(SSAORenderPass)).GetOcclusionTexture().GetSRV());
				fsPass.SetInputTarget(m_pFinalBlitInputIsIndex0 ? m_pCameraColor0->GetSRV() : m_pCameraColor1->GetSRV());
				break;
			case RendererView::TiledLighting:
				fsPass.SetInputTarget(m_pDebugTiledLighting->GetSRV());
				break;
			case RendererView::ClusteredLighting:
				fsPass.SetInputTarget(m_pDebugClusteredLighting->GetSRV());
				break;
			case RendererView::SSRTrace:
				fsPass.SetInputTarget(m_pDebugSSR->GetSRV());
				break;
			}

			gfx.SetViewport(screenWidth, screenHeight);
			gfx.SetRenderTarget(gfx.GetBackBufferView());

			pass.Execute(gfx, renderState);
			pass.UnbindSharedResources(gfx, renderState);
		}

		gfx.GetRenderStats().EndFrame();

	}

	void Renderer::DrawImguiControlWindow(const GraphicsDevice& gfx)
	{
		if (ImGui::Begin("Renderer"))
		{
			static const ImVec2 buttonSize = { 110, 25 };
			static const ImVec2 featureButtonSize = { 70, 18 };

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

			ImGui::BeginTable("FeatureTable", 2);
			int buttonId = 0;
			m_rendererFeatureEnabled[(int)RendererFeature::Shadows]			= DrawToggleOnOffButton(buttonId++, "Shadows", m_rendererFeatureEnabled[(int)RendererFeature::Shadows], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::DepthOfField]	= DrawToggleOnOffButton(buttonId++, "Depth of Field", m_rendererFeatureEnabled[(int)RendererFeature::DepthOfField], featureButtonSize, changed);
			if (IsFeatureEnabled(RendererFeature::DepthOfField))
			{
				GetRenderPass(DepthOfFieldRenderPass).DrawImguiControls(gfx);
			}
			m_rendererFeatureEnabled[(int)RendererFeature::Bloom]			= DrawToggleOnOffButton(buttonId++, "Bloom", m_rendererFeatureEnabled[(int)RendererFeature::Bloom], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::FXAA]			= DrawToggleOnOffButton(buttonId++, "FXAA", m_rendererFeatureEnabled[(int)RendererFeature::FXAA], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::SSAO]			= DrawToggleOnOffButton(buttonId++, "SSAO", m_rendererFeatureEnabled[(int)RendererFeature::SSAO], featureButtonSize, changed);
			if (IsFeatureEnabled(RendererFeature::SSAO))
			{
				GetRenderPass(SSAORenderPass).DrawImguiControls(gfx);
			}
			m_rendererFeatureEnabled[(int)RendererFeature::HZBSSR]			= DrawToggleOnOffButton(buttonId++, "SSR", m_rendererFeatureEnabled[(int)RendererFeature::HZBSSR], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::Dither]			= DrawToggleOnOffButton(buttonId++, "Dither", m_rendererFeatureEnabled[(int)RendererFeature::Dither], featureButtonSize, changed);
			m_rendererFeatureEnabled[(int)RendererFeature::Tonemapping]		= DrawToggleOnOffButton(buttonId++, "Tonemapping", m_rendererFeatureEnabled[(int)RendererFeature::Tonemapping], featureButtonSize, changed);
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

	RenderPass& Renderer::GetRenderPass(const RenderPassType pass) const
	{
		if (m_pRenderPasses.find(pass) == m_pRenderPasses.end()) THROW("Requested RenderPass does not exist!");
		return *m_pRenderPasses.at(pass).get();
	}

	const RenderPass& Renderer::CreateRenderPass(const RenderPassType pass)
	{
		if (m_pRenderPasses.find(pass) != m_pRenderPasses.end()) THROW("RenderPass cannot be created twice!");
		m_pRenderPasses.emplace(pass, std::move(std::make_unique<RenderPass>(pass)));
		return *m_pRenderPasses[pass].get();
	}

	const RenderPass& Renderer::CreateRenderPass(const RenderPassType pass, std::unique_ptr<RenderPass> pRenderPass)
	{
		if (m_pRenderPasses.find(pass) != m_pRenderPasses.end()) THROW("RenderPass cannot be created twice!");
		m_pRenderPasses.emplace(pass, std::move(pRenderPass));
		return *m_pRenderPasses[pass].get();
	}

	bool Renderer::IsFeatureEnabled(RendererFeature feature) const
	{
		return m_rendererFeatureEnabled[(int)feature];
	}
}