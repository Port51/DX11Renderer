#include "pch.h"
#include "BloomPass.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "RenderTexture.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Gaussian.h"
#include "ComputeShader.h"
#include "ComputeKernel.h"
#include "Binding.h"

namespace gfx
{

	BloomPass::BloomPass(const GraphicsDevice & gfx)
		: RenderPass(RenderPassType::BloomRenderPass)
	{
		const UINT bloomTextureWidth = gfx.GetScreenWidth() >> 1u;
		const UINT bloomTextureHeight = gfx.GetScreenHeight() >> 1u;

		CreateSubPass(BloomSubpass::PrefilterSubpass);
		CreateSubPass(BloomSubpass::SeparableBlurSubpass);
		CreateSubPass(BloomSubpass::CombineSubpass);

		m_pBloomTarget0 = std::make_shared<RenderTexture>(gfx);
		m_pBloomTarget0->Init(gfx.GetAdapter(), bloomTextureWidth, bloomTextureHeight);

		m_pBloomTarget1 = std::make_shared<RenderTexture>(gfx);
		m_pBloomTarget1->Init(gfx.GetAdapter(), bloomTextureWidth, bloomTextureHeight);

		m_pBloomCB = std::make_unique<ConstantBuffer>(gfx, D3D11_USAGE_DYNAMIC, sizeof(BloomCB));
		m_pBloomGaussianWeights = std::make_unique<StructuredBuffer>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, BloomBlurWidth * 2u + 1u, sizeof(f32));

		// todo: make this a setting that can be changed
		std::vector<f32> blurWeights;
		blurWeights.resize(BloomBlurWidth * 2u + 1u);
		Gaussian::GetGaussianWeights1D(blurWeights, 5.f);
		m_pBloomGaussianWeights->Update(gfx, blurWeights.data());

		m_pBloomPrefilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Bloom.hlsl", "Prefilter"));
		m_pBloomHorizontalBlurKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Bloom.hlsl", "HorizontalGaussian"));
		m_pBloomVerticalBlurKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Bloom.hlsl", "VerticalGaussian"));
		m_pBloomCombineKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "Bloom.hlsl", "Combine"));

	}

	void BloomPass::SetupRenderPassDependencies(const GraphicsDevice & gfx, const RenderTexture & pDownsampledColor, const RenderTexture & pCameraColor)
	{
		GetSubPass(BloomSubpass::PrefilterSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pDownsampledColor.GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pBloomTarget0->GetUAV());

		GetSubPass(BloomSubpass::SeparableBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pBloomGaussianWeights->GetSRV());

		GetSubPass(BloomSubpass::CombineSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pBloomTarget0->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pCameraColor.GetUAV());
	}

	void BloomPass::Execute(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		gfx.GetRenderStats().StartTaskTimer(GetName());

		auto context = gfx.GetContext();
		const UINT screenWidth = gfx.GetScreenWidth();
		const UINT screenHeight = gfx.GetScreenHeight();
		const UINT bloomTextureWidth = screenWidth >> 1u;
		const UINT bloomTextureHeight = screenHeight >> 1u;

		// Bloom prefilter
		{
			const RenderPass& pass = GetSubPass(BloomSubpass::PrefilterSubpass);
			pass.BindSharedResources(gfx, renderState);

			static BloomCB bloomCB;
			bloomCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			m_pBloomCB->Update(gfx, &bloomCB);

			m_pBloomPrefilterKernel->Dispatch(gfx, bloomTextureWidth, bloomTextureHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Bloom blur (x2 sub-passes)
		{
			const RenderPass& pass = GetSubPass(BloomSubpass::SeparableBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			static BloomCB bloomCB;
			bloomCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			m_pBloomCB->Update(gfx, &bloomCB);

			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, m_pBloomTarget0->GetSRV().GetAddressOf());
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pBloomGaussianWeights->GetSRV().GetAddressOf());
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, m_pBloomTarget1->GetUAV().GetAddressOf(), nullptr);
			m_pBloomHorizontalBlurKernel->Dispatch(gfx, bloomTextureWidth, bloomTextureHeight, 1u);
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, RenderConstants::NullSRVArray.data());
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, RenderConstants::NullUAVArray.data(), nullptr);

			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, m_pBloomTarget1->GetSRV().GetAddressOf());
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, m_pBloomTarget0->GetUAV().GetAddressOf(), nullptr);
			m_pBloomVerticalBlurKernel->Dispatch(gfx, bloomTextureWidth, bloomTextureHeight, 1u);
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, RenderConstants::NullSRVArray.data());
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, RenderConstants::NullUAVArray.data(), nullptr);

			pass.UnbindSharedResources(gfx, renderState);
			gfx.GetRenderStats().EndTaskTimer(GetName());
		}

		// Bloom combine
		{
			const RenderPass& pass = GetSubPass(BloomSubpass::CombineSubpass);
			pass.BindSharedResources(gfx, renderState);

			static BloomCB bloomCB;
			bloomCB.resolutionSrcDst = { screenWidth, screenHeight, screenWidth, screenHeight };
			m_pBloomCB->Update(gfx, &bloomCB);

			m_pBloomCombineKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}
	}

}