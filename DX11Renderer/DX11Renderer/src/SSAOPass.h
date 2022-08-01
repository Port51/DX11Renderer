#pragma once
#include "RenderPass.h"
#include "CommonHeader.h"

namespace gfx
{
	class Bindable;
	class VertexBufferWrapper;
	class Texture;
	class VertexShader;
	class ComputeKernel;
	class RenderTexture;
	class DepthStencilTarget;
	class Sampler;
	class RandomGenerator;

	struct SSAO_CB;

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	class SSAOPass : public RenderPass
	{
	private:
		enum SSAOSubpass : UINT {
			OcclusionSubpass,
			HorizontalBlurSubpass,
			VerticalBlurSubpass,
		};
	public:
		SSAOPass(const GraphicsDevice & gfx, RandomGenerator& rng);

	public:
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const RenderTexture & pGbuffer, const RenderTexture& hiZBuffer, const Texture& noiseTexture);
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const override;
		void DrawImguiControls(const GraphicsDevice& gfx) override;
		const RenderTexture& GetOcclusionTexture() const;

	private:
		std::unique_ptr<ConstantBuffer<SSAO_CB>> m_pSettingsCB;
		std::unique_ptr<SSAO_CB> m_pSettings;
		std::unique_ptr<StructuredBuffer<float>> m_pGaussianBlurWeights;

		std::unique_ptr<ComputeKernel> m_pOcclusionKernel;
		std::unique_ptr<ComputeKernel> m_pHorizontalBlurKernel;
		std::unique_ptr<ComputeKernel> m_pVerticalBlurKernel;
		std::unique_ptr<StructuredBuffer<dx::XMVECTOR>> m_pSampleOffsetSB;

		// Use x2 occlusion textures because of separable blur
		std::shared_ptr<RenderTexture> m_pOcclusionTexture0;
		std::shared_ptr<RenderTexture> m_pOcclusionTexture1;

		float m_radiusVS = 0.15f;
		float m_biasVS = 0.001f;
		float m_intensity = 2.5f;
		float m_sharpness = 0.75f;

		const size_t SampleOffsetCount = 64u;
		const size_t BlurWidth = 15u;
	};
}