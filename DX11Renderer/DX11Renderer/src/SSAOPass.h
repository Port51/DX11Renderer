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

	struct DepthOfFieldCB;

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
		void Execute(const GraphicsDevice& gfx) const override;
		const RenderTexture& GetOcclusionTexture() const;

	private:
		std::unique_ptr<ComputeKernel> m_pOcclusionKernel;
		std::unique_ptr<ComputeKernel> m_pHorizontalBlurKernel;
		std::unique_ptr<ComputeKernel> m_pVerticalBlurKernel;
		std::unique_ptr<StructuredBuffer<dx::XMVECTOR>> m_pSampleOffsetSB;

		// Use x2 occlusion textures because of separable blur
		std::shared_ptr<RenderTexture> m_pOcclusionTexture0;
		std::shared_ptr<RenderTexture> m_pOcclusionTexture1;

		const size_t SampleOffsetCount = 64u;
	};
}