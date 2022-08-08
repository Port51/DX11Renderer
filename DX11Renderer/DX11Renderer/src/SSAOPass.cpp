#include "pch.h"
#include "SSAOPass.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "RenderTexture.h"
#include "DepthStencilTarget.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Gaussian.h"
#include "ComputeShader.h"
#include "ComputeKernel.h"
#include "Binding.h"
#include "RandomGenerator.h"
#include "CommonCbuffers.h"

namespace gfx
{

	SSAOPass::SSAOPass(const GraphicsDevice & gfx, RandomGenerator& rng)
		: RenderPass(RenderPassType::SSAORenderPass)
	{
		const UINT occlusionTextureWidth = gfx.GetScreenWidth();
		const UINT occlusionTextureHeight = gfx.GetScreenHeight();

		CreateSubPass(SSAOSubpass::OcclusionSubpass);
		CreateSubPass(SSAOSubpass::HorizontalBlurSubpass);
		CreateSubPass(SSAOSubpass::VerticalBlurSubpass);

		m_pOcclusionTexture0 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R8_UNORM);
		m_pOcclusionTexture0->Init(gfx.GetAdapter(), occlusionTextureWidth, occlusionTextureHeight);

		m_pOcclusionTexture1 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R8_UNORM);
		m_pOcclusionTexture1->Init(gfx.GetAdapter(), occlusionTextureWidth, occlusionTextureHeight);

		m_pOcclusionKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "SSAO.hlsl", "OcclusionPass"));
		m_pHorizontalBlurKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "SSAO.hlsl", "HorizontalBlurPass"));
		m_pVerticalBlurKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, "SSAO.hlsl", "VerticalBlurPass"));

		// Create random sample directions
		std::vector<dx::XMVECTOR> sampleOffsets;
		sampleOffsets.reserve(SampleOffsetCount);
		for (size_t i = 0; i < SampleOffsetCount; ++i)
		{
			// Get random hemisphere direction
			sampleOffsets.emplace_back(dx::XMVector3Normalize(dx::XMVectorSet(
				rng.GetUniformFloat01() * 2.f - 1.f,
				rng.GetUniformFloat01() * 2.f - 1.f,
				rng.GetUniformFloat01(),
				0.f
			)));
		}
		m_pSampleOffsetSB = std::make_unique<StructuredBuffer<dx::XMVECTOR>>(gfx, D3D11_USAGE::D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, SampleOffsetCount, sampleOffsets.data(), false);

		m_pGaussianBlurWeights = std::make_unique<StructuredBuffer<f32>>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, BlurWidth * 2u + 1u);

		// todo: make this a setting that can be changed
		std::vector<f32> blurWeights;
		blurWeights.resize(BlurWidth * 2u + 1u);
		Gaussian::GetGaussianWeights1D(blurWeights, 5.f);
		m_pGaussianBlurWeights->Update(gfx, blurWeights, blurWeights.size());

		m_pSettings = std::make_unique<SSAO_CB>();
		m_pSettingsCB = std::make_unique<ConstantBuffer<SSAO_CB>>(gfx, D3D11_USAGE_DYNAMIC);
	}

	void SSAOPass::SetupRenderPassDependencies(const GraphicsDevice & gfx, const RenderTexture & pGbuffer, const RenderTexture& hiZBuffer, const Texture& noiseTexture)
	{
		GetSubPass(SSAOSubpass::OcclusionSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pGbuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, hiZBuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, m_pSampleOffsetSB->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, noiseTexture.GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pOcclusionTexture0->GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pSettingsCB->GetD3DBuffer());

		GetSubPass(SSAOSubpass::HorizontalBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, hiZBuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, m_pOcclusionTexture0->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 5u, m_pGaussianBlurWeights->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pOcclusionTexture1->GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pSettingsCB->GetD3DBuffer());

		GetSubPass(SSAOSubpass::VerticalBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, hiZBuffer.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, m_pOcclusionTexture1->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 5u, m_pGaussianBlurWeights->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pOcclusionTexture0->GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_pSettingsCB->GetD3DBuffer());
	}

	void SSAOPass::Execute(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		// Update settings
		{
			m_pSettings->radiusVS = m_radiusVS;
			m_pSettings->biasVS = m_biasVS;
			m_pSettings->intensity = m_intensity;
			m_pSettings->sharpness = m_sharpness;
			m_pSettingsCB->Update(gfx, *m_pSettings);
		}

		// Render occlusion
		{
			const RenderPass& pass = GetSubPass(SSAOSubpass::OcclusionSubpass);
			pass.BindSharedResources(gfx, renderState);

			m_pOcclusionKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Horizontal blur
		{
			const RenderPass& pass = GetSubPass(SSAOSubpass::HorizontalBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			m_pHorizontalBlurKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// Vertical blur
		{
			const RenderPass& pass = GetSubPass(SSAOSubpass::VerticalBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			m_pVerticalBlurKernel->Dispatch(gfx, gfx.GetScreenWidth(), gfx.GetScreenHeight(), 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}
	}

	void SSAOPass::DrawImguiControls(const GraphicsDevice & gfx)
	{
		int guiId = 17390;
		const int indent = 50;
		DrawSliderFloat(guiId++, indent, "Radius", &m_radiusVS, 0.001f, 0.25f, "%.3f", ImGuiSliderFlags_None);
		DrawSliderFloat(guiId++, indent, "Bias", &m_biasVS, 0.001f, 0.005f, "%.3f", ImGuiSliderFlags_None);
		DrawSliderFloat(guiId++, indent, "Intensity", &m_intensity, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_None);
		DrawSliderFloat(guiId++, indent, "Sharpness", &m_sharpness, 0.3f, 2.0f, "%.3f", ImGuiSliderFlags_None);
	}

	const RenderTexture & SSAOPass::GetOcclusionTexture() const
	{
		return *m_pOcclusionTexture0;
	}

}