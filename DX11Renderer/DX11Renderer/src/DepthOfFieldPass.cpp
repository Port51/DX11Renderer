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
#include "Sampler.h"

namespace gfx
{

	DepthOfFieldPass::DepthOfFieldPass(const GraphicsDevice & gfx, DepthOfFieldBokehType bokehType)
		: RenderPass(RenderPassType::DepthOfFieldRenderPass), m_bokehType(bokehType)
	{
		const UINT dofTextureWidth = gfx.GetScreenWidth() >> 1u;
		const UINT dofTextureHeight = gfx.GetScreenHeight() >> 1u;

		CreateSubPass(DepthOfFieldSubpass::PrefilterSubpass);
		CreateSubPass(DepthOfFieldSubpass::FarBlurSubpass);
		CreateSubPass(DepthOfFieldSubpass::NearBlurSubpass);
		CreateSubPass(DepthOfFieldSubpass::CompositeSubpass);

		const char* computeShaderPath = "DepthOfField.hlsl";
		m_pDoFPrefilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "Prefilter"));

		m_pDoFHorizontalFilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "HorizontalFilter"));
		m_pDoFVerticalFilterAndCombineKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "VerticalFilterAndCombine"));
		m_pDoFCompositeKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "Composite"));

		m_pVerticalHexFilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "VerticalHexFilter"));
		m_pDiagonalHexFilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "DiagonalHexFilter"));
		m_pRhomboidHexFilterKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "RhomboidHexFilter"));

		m_pBokehDiskWeights = std::make_unique<StructuredBuffer<f32>>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, BokehDiskComponentElements * 6u);

		// First 2 bokeh weights are for near CoC, next 4 are for the 2 components of far CoC
		// Using functions and settings from http://yehar.com/blog/?p=1495
		std::vector<f32> bokehWeights;
		bokehWeights.resize(BokehDiskComponentElements * 6u);
		const float bokehXScale = 1.f / (float)BokehDiskWidth; // scale blur radius

		// Near C0
		Bokeh::GetDiskRealWeights1D(bokehWeights, 0u, BokehDiskComponentElements, 0.862325f, 1.624835f, bokehXScale, false);
		Bokeh::GetDiskImaginaryWeights1D(bokehWeights, 1u, BokehDiskComponentElements, 0.862325f, 1.624835f, bokehXScale, false);

		// Far C0
		Bokeh::GetDiskRealWeights1D(bokehWeights, 2u, BokehDiskComponentElements, 0.886528f, 5.268909f, bokehXScale, false);
		Bokeh::GetDiskImaginaryWeights1D(bokehWeights, 3u, BokehDiskComponentElements, 0.886528f, 5.268909f, bokehXScale, false);

		// Far C1
		Bokeh::GetDiskRealWeights1D(bokehWeights, 4u, BokehDiskComponentElements, 1.960518f, 1.558213f, bokehXScale, false);
		Bokeh::GetDiskImaginaryWeights1D(bokehWeights, 5u, BokehDiskComponentElements, 1.960518f, 1.558213f, bokehXScale, false);

		// Normalize weights
		// Keep in mind that weights are applied twice, so use sqrt when normalizing
		const float accuNear = Bokeh::GetDiskAccumulation(bokehWeights, 0u, 1u, BokehDiskComponentElements, 0.767583f, 1.862321f);
		Bokeh::ApplyScaleToDisk(bokehWeights, 0u, 1u, BokehDiskComponentElements, 1.f / std::sqrt(accuNear));
		const float accuFar0 = Bokeh::GetDiskAccumulation(bokehWeights, 1u, 1u, BokehDiskComponentElements, 0.411259f, -0.548794f);
		const float accuFar1 = Bokeh::GetDiskAccumulation(bokehWeights, 1u, 1u, BokehDiskComponentElements, 0.513282f, 4.561110f);
		const float farScale = 1.f / std::sqrt(accuFar0 + accuFar1);
		Bokeh::ApplyScaleToDisk(bokehWeights, 1u, 2u, BokehDiskComponentElements, farScale);

		m_depthOfFieldCB = std::make_unique<DepthOfFieldCB>();

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

		// Only used for hex bokehs
		m_pDoFNear3 = std::make_shared<RenderTexture>(gfx, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_pDoFNear3->Init(gfx.GetAdapter(), dofTextureWidth, dofTextureHeight);

		m_pClampedMaxSampler = std::make_unique<Sampler>(gfx, D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);
	}

	void DepthOfFieldPass::SetupRenderPassDependencies(const GraphicsDevice & gfx, const RenderTexture& pDownsampledColor, const RenderTexture& pHiZBufferTarget, const RenderTexture& pCameraColor)
	{
		GetSubPass(DepthOfFieldSubpass::PrefilterSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, pDownsampledColor.GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 3u, pHiZBufferTarget.GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDoFFar0->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDoFNear0->GetUAV());

		GetSubPass(DepthOfFieldSubpass::FarBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pDoFFar0->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, m_pBokehDiskWeights->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDoFFar1->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDoFFar2->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, m_pDoFFar3->GetUAV());

		GetSubPass(DepthOfFieldSubpass::NearBlurSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pDoFNear0->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 4u, m_pBokehDiskWeights->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_pDoFNear1->GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_pDoFNear2->GetUAV());

		GetSubPass(DepthOfFieldSubpass::CompositeSubpass).
			ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_pDoFFar3->GetSRV())
			.CSSetSRV(RenderSlots::CS_FreeSRV + 1u, m_pDoFNear0->GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, pCameraColor.GetUAV());

	}

	void DepthOfFieldPass::DrawImguiControls(const GraphicsDevice & gfx)
	{
		int guiId = 12390;
		const int indent = 50;
		DrawSliderFloat(guiId++, indent, "Focus Distance", &m_focusDistance, 1.0f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		DrawSliderFloat(guiId++, indent, "Focus Width", &m_focusWidth, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		DrawSliderFloat(guiId++, indent, "Far Fade Width", &m_farFadeWidth, 0.01f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		DrawSliderFloat(guiId++, indent, "Near Fade Width", &m_nearFadeWidth, 0.01f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		DrawSliderFloat(guiId++, indent, "Far Intensity", &m_farIntensity, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_None);
		DrawSliderFloat(guiId++, indent, "Near Intensity", &m_nearIntensity, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_None);
	}

	void DepthOfFieldPass::Execute(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		auto context = gfx.GetContext();
		const UINT screenWidth = gfx.GetScreenWidth();
		const UINT screenHeight = gfx.GetScreenHeight();
		const UINT dofTextureWidth = screenWidth >> 1u;
		const UINT dofTextureHeight = screenHeight >> 1u;

		// Run same prefilter pass for all bokehs

		// Precalculate CoC factors
		const float limitedNearFadeWidth = std::max(0.01f, m_nearFadeWidth);
		m_depthOfFieldCB->nearCoCScale = -1.f / limitedNearFadeWidth;
		m_depthOfFieldCB->nearCoCBias = (m_focusDistance - m_focusWidth) / limitedNearFadeWidth;
		m_depthOfFieldCB->nearCoCIntensity = m_nearIntensity;

		const float limitedFarFadeWidth = std::max(0.01f, m_farFadeWidth);
		m_depthOfFieldCB->farCoCScale = 1.f / limitedFarFadeWidth;
		m_depthOfFieldCB->farCoCBias = -(m_focusDistance + m_focusWidth) / limitedFarFadeWidth;
		m_depthOfFieldCB->farCoCIntensity = m_farIntensity;

		// DoF prefilter
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::PrefilterSubpass);
			pass.BindSharedResources(gfx, renderState);

			m_pDepthOfFieldCB->Update(gfx, *m_depthOfFieldCB);
			context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());

			m_pDoFPrefilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		switch (m_bokehType)
		{
		case DepthOfFieldBokehType::DiskBokeh:
			ExecuteDiskBokeh(gfx, renderState);
			break;
		case DepthOfFieldBokehType::HexBokeh:
			ExecuteHexBokeh(gfx, renderState);
			break;
		default:
			THROW("Unrecognized bokeh type " + m_bokehType);
		}
	}

	void DepthOfFieldPass::ExecuteDiskBokeh(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		auto context = gfx.GetContext();
		UINT screenWidth = gfx.GetScreenWidth();
		UINT screenHeight = gfx.GetScreenHeight();
		UINT dofTextureWidth = screenWidth >> 1u;
		UINT dofTextureHeight = screenHeight >> 1u;

		// DoF far blur pass
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::FarBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			// Component #0
			{
				m_depthOfFieldCB->weightOffset = BokehDiskComponentElements * 2u;
				m_depthOfFieldCB->verticalPassAddFactor = 0.f;
				m_depthOfFieldCB->combineRealFactor = 0.411259f;
				m_depthOfFieldCB->combineImaginaryFactor = -0.548794f;
				m_pDepthOfFieldCB->Update(gfx, *m_depthOfFieldCB);
				context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());
				REGISTER_GPU_CALL();

				// Input = CoC
				// Outputs = real + imag
				m_pDoFHorizontalFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

				// Inputs = real + imag
				// Output = combined texture

				// Remap UAVs to SRVs
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, RenderConstants::NullUAVArray.data(), nullptr);
				context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pDoFFar1->GetSRV().GetAddressOf());
				context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 2u, 1u, m_pDoFFar2->GetSRV().GetAddressOf());
				REGISTER_GPU_CALLS(3u);

				m_pDoFVerticalFilterAndCombineKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);
			}

			// Component #1
			// todo: don't set these again!
			{
				m_depthOfFieldCB->weightOffset = BokehDiskComponentElements * 4u;
				m_depthOfFieldCB->verticalPassAddFactor = 1.f;
				m_depthOfFieldCB->combineRealFactor = 0.513282f;
				m_depthOfFieldCB->combineImaginaryFactor = 4.561110f;
				m_pDepthOfFieldCB->Update(gfx, *m_depthOfFieldCB);
				context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());
				REGISTER_GPU_CALL();

				// Input = CoC
				// Outputs = real + imag

				// Remap SRVs to UAVs
				context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 2u, RenderConstants::NullSRVArray.data());
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 1u, m_pDoFFar1->GetUAV().GetAddressOf(), nullptr);
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 1u, 1u, m_pDoFFar2->GetUAV().GetAddressOf(), nullptr);
				REGISTER_GPU_CALLS(3u);

				m_pDoFHorizontalFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

				// Inputs = real + imag
				// Output = combined texture

				// Remap UAVs to SRVs
				context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, RenderConstants::NullUAVArray.data(), nullptr);
				context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pDoFFar1->GetSRV().GetAddressOf());
				context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 2u, 1u, m_pDoFFar2->GetSRV().GetAddressOf());
				REGISTER_GPU_CALLS(3u);

				m_pDoFVerticalFilterAndCombineKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);
			}

			pass.UnbindSharedResources(gfx, renderState);
		}

		// DoF near blur pass
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::NearBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			m_depthOfFieldCB->weightOffset = 0u;
			m_depthOfFieldCB->verticalPassAddFactor = 0.f;
			m_depthOfFieldCB->combineRealFactor = 0.767583f;
			m_depthOfFieldCB->combineImaginaryFactor = 1.862321f;
			m_pDepthOfFieldCB->Update(gfx, *m_depthOfFieldCB);
			context->CSSetConstantBuffers(RenderSlots::CS_FreeCB + 0u, 1u, m_pDepthOfFieldCB->GetD3DBuffer().GetAddressOf());
			REGISTER_GPU_CALL();

			// Input = CoC
			// Outputs = real + imag
			m_pDoFHorizontalFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			// Inputs = real + imag
			// Output = combined texture

			// Remap UAVs to SRVs
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, RenderConstants::NullUAVArray.data(), nullptr);
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pDoFNear1->GetSRV().GetAddressOf());
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 2u, 1u, m_pDoFNear2->GetSRV().GetAddressOf());
			REGISTER_GPU_CALLS(3u);

			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 2u, 1u, m_pDoFNear0->GetUAV().GetAddressOf(), nullptr);
			REGISTER_GPU_CALLS(2u);
			m_pDoFVerticalFilterAndCombineKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 2u, 1u, RenderConstants::NullUAVArray.data(), nullptr);
			REGISTER_GPU_CALL();

			pass.UnbindSharedResources(gfx, renderState);
		}

		// DoF composite pass
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::CompositeSubpass);
			pass.BindSharedResources(gfx, renderState);

			m_pDoFCompositeKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}
	}

	void DepthOfFieldPass::ExecuteHexBokeh(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		auto context = gfx.GetContext();
		UINT screenWidth = gfx.GetScreenWidth();
		UINT screenHeight = gfx.GetScreenHeight();
		UINT dofTextureWidth = screenWidth >> 1u;
		UINT dofTextureHeight = screenHeight >> 1u;

		// DoF far blur pass
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::FarBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			// todo: move to shared
			context->CSSetSamplers(0u, 1u, m_pClampedMaxSampler->GetD3DSampler().GetAddressOf());

			// Input = CoC
			// Outputs = vert blur
			m_pVerticalHexFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			// Inputs = CoC
			// Output = diagonal blur
			m_pDiagonalHexFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, RenderConstants::NullUAVArray.data(), nullptr);
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, m_pDoFFar1->GetSRV().GetAddressOf());
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pDoFFar2->GetSRV().GetAddressOf());

			// Inputs = vert, diag
			// Output = composite
			m_pRhomboidHexFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// DoF near blur pass
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::NearBlurSubpass);
			pass.BindSharedResources(gfx, renderState);

			// todo: move to shared
			context->CSSetSamplers(0u, 1u, m_pClampedMaxSampler->GetD3DSampler().GetAddressOf());

			// Input = CoC
			// Outputs = vert blur
			m_pVerticalHexFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			// Inputs = CoC
			// Output = diagonal blur
			m_pDiagonalHexFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 2u, RenderConstants::NullUAVArray.data(), nullptr);
			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 2u, 1u, m_pDoFNear3->GetUAV().GetAddressOf(), nullptr);
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, m_pDoFNear1->GetSRV().GetAddressOf());
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pDoFNear2->GetSRV().GetAddressOf());

			// Inputs = vert, diag
			// Output = composite
			m_pRhomboidHexFilterKernel->Dispatch(gfx, dofTextureWidth, dofTextureHeight, 1u);

			context->CSSetUnorderedAccessViews(RenderSlots::CS_FreeUAV + 0u, 3u, RenderConstants::NullUAVArray.data(), nullptr);

			pass.UnbindSharedResources(gfx, renderState);
		}

		// DoF composite pass
		{
			const RenderPass& pass = GetSubPass(DepthOfFieldSubpass::CompositeSubpass);
			pass.BindSharedResources(gfx, renderState);

			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 0u, 1u, m_pDoFFar3->GetSRV().GetAddressOf());
			context->CSSetShaderResources(RenderSlots::CS_FreeSRV + 1u, 1u, m_pDoFNear3->GetSRV().GetAddressOf());

			m_pDoFCompositeKernel->Dispatch(gfx, screenWidth, screenHeight, 1u);

			pass.UnbindSharedResources(gfx, renderState);
		}
	}

	void DepthOfFieldPass::SetOutputTarget(std::shared_ptr<Texture> pTarget)
	{

	}

}