#include "pch.h"
#include "RenderConstants.h"
#include "DepthOfFieldPass.h"
#include "GraphicsDevice.h"
#include "Texture.h"
#include "Binding.h"
#include "Bokeh.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "RenderTexture.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "CommonCbuffers.h"

namespace gfx
{

	DepthOfFieldPass::DepthOfFieldPass(const GraphicsDevice & gfx)
		: RenderPass(RenderPassType::DoFPass)
	{
		UINT dofTextureWidth = (UINT)gfx.GetScreenWidth() >> 1u;
		UINT dofTextureHeight = (UINT)gfx.GetScreenHeight() >> 1u;

		CreateSubPass(RenderPassType::DoFPrefilterSubpass);
		CreateSubPass(RenderPassType::DoFFarBlurSubpass);
		CreateSubPass(RenderPassType::DoFNearBlurSubpass);
		CreateSubPass(RenderPassType::DoFCompositeSubpass);

		const char* computeShaderPath = "Assets\\Shaders\\DepthOfField.hlsl";
		m_pDoFPrefilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "Prefilter"));
		m_pDoFHorizontalFilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "HorizontalFilter"));
		m_pDoFVerticalFilterAndCombineKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "VerticalFilterAndCombine"));
		m_pDoFCompositeKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "Composite"));

		m_pBokehDiskWeights = std::make_unique<StructuredBuffer<f32>>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, BokehDiskComponentElements * 6u);

		// First 2 bokeh weights are for near CoC, next 4 are for the 2 components of far CoC
		// Using functions and settings from http://yehar.com/blog/?p=1495
		std::vector<f32> bokehWeights;
		bokehWeights.resize(BokehDiskComponentElements * 6u);
		const float bokehXScale = 2.5f / (float)BokehDiskWidth; // scale blur radius
		Bokeh::GetDiskRealWeights1D(bokehWeights, 0u, BokehDiskComponentElements, 0.862325f, 1.624835f, bokehXScale, true);
		Bokeh::GetDiskImaginaryWeights1D(bokehWeights, 1u, BokehDiskComponentElements, 0.862325f, 1.624835f, bokehXScale, true);
		Bokeh::GetDiskRealWeights1D(bokehWeights, 2u, BokehDiskComponentElements, 0.886528f, 5.268909f, bokehXScale, true);
		Bokeh::GetDiskImaginaryWeights1D(bokehWeights, 3u, BokehDiskComponentElements, 0.886528f, 5.268909f, bokehXScale, true);
		Bokeh::GetDiskRealWeights1D(bokehWeights, 4u, BokehDiskComponentElements, 1.960518f, 1.558213f, bokehXScale, true);
		Bokeh::GetDiskImaginaryWeights1D(bokehWeights, 5u, BokehDiskComponentElements, 1.960518f, 1.558213f, bokehXScale, true);

		const float accuNear = Bokeh::GetDiskAccumulation(bokehWeights, 0u, 1u, BokehDiskComponentElements, 0.767583f, 1.862321f);
		Bokeh::ApplyScaleToDisk(bokehWeights, 0u, 1u, BokehDiskComponentElements, 1.f / std::sqrt(accuNear));
		const float accuFar0 = Bokeh::GetDiskAccumulation(bokehWeights, 1u, 1u, BokehDiskComponentElements, 0.411259f, -0.548794f);
		const float accuFar1 = Bokeh::GetDiskAccumulation(bokehWeights, 1u, 1u, BokehDiskComponentElements, 0.513282f, 4.561110f);
		const float farScale = 1.f / std::pow(accuFar0 + accuFar1, 0.5f);
		Bokeh::ApplyScaleToDisk(bokehWeights, 1u, 2u, BokehDiskComponentElements, farScale);

		m_pDepthOfFieldCB = std::make_unique<ConstantBuffer<DepthOfFieldCB>>(gfx, D3D11_USAGE_DYNAMIC);
		m_pBokehDiskWeights->Update(gfx, bokehWeights, bokehWeights.size());

		m_pDoFFar0 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFFar0->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pDoFFar1 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFFar1->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pDoFFar2 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFFar2->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pDoFFar3 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFFar3->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pDoFNear0 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFNear0->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pDoFNear1 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFNear1->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pDoFNear2 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFNear2->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);
	}

	void DepthOfFieldPass::SetupRenderPassDependencies(const GraphicsDevice & gfx, const RenderTexture& pDownsampledColor, const RenderTexture& pHiZBufferTarget, const RenderTexture& pCameraColor)
	{
		GetSubPass(RenderPassType::DoFPrefilterSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pDownsampledColor.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 2u, pHiZBufferTarget.GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDoFFar0->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDoFNear0->GetUAV());

		GetSubPass(RenderPassType::DoFFarBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pDoFFar0->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, m_pBokehDiskWeights->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDoFFar1->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDoFFar2->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, m_pDoFFar3->GetUAV());

		GetSubPass(RenderPassType::DoFNearBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pDoFNear0->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, m_pBokehDiskWeights->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDoFNear1->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDoFNear2->GetUAV());

		GetSubPass(RenderPassType::DoFCompositeSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pDoFFar3->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pDoFNear0->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pCameraColor.GetUAV());

	}

	void DepthOfFieldPass::DrawImguiControls(const GraphicsDevice & gfx)
	{
		ImGui::SliderFloat("Focus Distance", &m_focusDistance, 1.0f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Focus Width", &m_focusWidth, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Near Intensity", &m_nearIntensity, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_None);
		ImGui::SliderFloat("Near Fade Width", &m_nearFadeWidth, 0.01f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Far Intensity", &m_farIntensity, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_None);
		ImGui::SliderFloat("Far Fade Width", &m_farFadeWidth, 0.01f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
	}

	void DepthOfFieldPass::Execute(const GraphicsDevice & gfx) const
	{
		auto context = gfx.GetContext();
		UINT screenWidth = (UINT)gfx.GetScreenWidth();
		UINT screenHeight = (UINT)gfx.GetScreenHeight();

		static DepthOfFieldCB depthOfFieldCB;

		// Precalculate CoC factors
		const float limitedNearFadeWidth = std::max(0.01f, m_nearFadeWidth);
		depthOfFieldCB.nearCoCScale = -1.f / limitedNearFadeWidth;
		depthOfFieldCB.nearCoCBias = (m_focusDistance - m_focusWidth) / limitedNearFadeWidth;
		depthOfFieldCB.nearCoCIntensity = m_nearIntensity;
		
		const float limitedFarFadeWidth = std::max(0.01f, m_farFadeWidth);
		depthOfFieldCB.farCoCScale = 1.f / limitedFarFadeWidth;
		depthOfFieldCB.farCoCBias = -(m_focusDistance + m_focusWidth) / limitedFarFadeWidth;
		depthOfFieldCB.farCoCIntensity = m_farIntensity;

		// DoF prefilter
		{
			const RenderPass& pass = GetSubPass(DoFPrefilterSubpass);
			pass.BindSharedResources(gfx);

			m_pDepthOfFieldCB->Update(gfx, depthOfFieldCB);
			context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());

			m_pDoFPrefilterKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);

			pass.UnbindSharedResources(gfx);
		}

		// DoF far blur pass
		{
			const RenderPass& pass = GetSubPass(DoFFarBlurSubpass);
			pass.BindSharedResources(gfx);

			// Component #0
			{
				depthOfFieldCB.weightOffset = BokehDiskComponentElements * 2u;
				depthOfFieldCB.verticalPassAddFactor = 0.f;
				depthOfFieldCB.combineRealFactor = 0.411259f;
				depthOfFieldCB.combineImaginaryFactor = -0.548794f;
				m_pDepthOfFieldCB->Update(gfx, depthOfFieldCB);
				context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());

				// Input = CoC
				// Outputs = real + imag
				m_pDoFHorizontalFilterKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);

				// Inputs = real + imag
				// Output = combined texture
				m_pDoFVerticalFilterAndCombineKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);
			}

			// Component #1
			// todo: don't set these again!
			{
				depthOfFieldCB.weightOffset = BokehDiskComponentElements * 4u;
				depthOfFieldCB.verticalPassAddFactor = 1.f;
				depthOfFieldCB.combineRealFactor = 0.513282f;
				depthOfFieldCB.combineImaginaryFactor = 4.561110f;
				m_pDepthOfFieldCB->Update(gfx, depthOfFieldCB);
				context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());

				// Input = CoC
				// Outputs = real + imag
				m_pDoFHorizontalFilterKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);

				// Inputs = real + imag
				// Output = combined texture
				m_pDoFVerticalFilterAndCombineKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);
			}

			pass.UnbindSharedResources(gfx);
		}

		// DoF near blur pass
		{
			const RenderPass& pass = GetSubPass(DoFNearBlurSubpass);
			pass.BindSharedResources(gfx);

			depthOfFieldCB.weightOffset = 0u;
			depthOfFieldCB.verticalPassAddFactor = 0.f;
			depthOfFieldCB.combineRealFactor = 0.767583f;
			depthOfFieldCB.combineImaginaryFactor = 1.862321f;
			m_pDepthOfFieldCB->Update(gfx, depthOfFieldCB);
			context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());

			// Input = CoC
			// Outputs = real + imag
			m_pDoFHorizontalFilterKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);

			// Inputs = real + imag
			// Output = combined texture
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, m_pNullSRVs.data());
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 2u, 1u, m_pDoFNear0->GetUAV().GetAddressOf(), nullptr);
			m_pDoFVerticalFilterAndCombineKernel->Dispatch(gfx, screenWidth >> 1u, screenHeight >> 1u, 1u);
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 2u, 1u, m_pNullUAVs.data(), nullptr);

			pass.UnbindSharedResources(gfx);
		}

		// DoF composite pass
		{
			const RenderPass& pass = GetSubPass(DoFCompositeSubpass);
			pass.BindSharedResources(gfx);

			m_pDoFCompositeKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx);
		}
	}

	void DepthOfFieldPass::SetOutputTarget(std::shared_ptr<Texture> pTarget)
	{

	}

}