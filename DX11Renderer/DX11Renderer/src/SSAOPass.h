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
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const RenderTexture & pGbuffer, const RenderTexture& pCameraColor);
		void Execute(const GraphicsDevice& gfx) const override;

	private:
		std::unique_ptr<ComputeKernel> m_pSSAOKernel;
		std::unique_ptr<StructuredBuffer<dx::XMVECTOR>> m_pSampleOffsetSB;
	};
}