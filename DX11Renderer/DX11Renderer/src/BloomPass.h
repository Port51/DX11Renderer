#pragma once
#include "RenderPass.h"
#include "CommonHeader.h"
#include "CommonCbuffers.h"

namespace gfx
{
	class Bindable;
	class VertexBufferWrapper;
	class Texture;
	class VertexShader;
	class ComputeKernel;
	class RenderTexture;
	class Sampler;
	class ConstantBuffer;
	class StructuredBuffer;

	struct DepthOfFieldCB;

	class BloomPass : public RenderPass
	{
	private:
		enum class BloomSubpass : u8 {
			PrefilterSubpass,
			SeparableBlurSubpass,
			CombineSubpass,
		};
	public:
		BloomPass(const GraphicsDevice& gfx);

	public:
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const RenderTexture& pDownsampledColor, const RenderTexture& pCameraColor);
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const override;

	private:
		std::unique_ptr<ConstantBuffer> m_pBloomCB;
		std::unique_ptr<StructuredBuffer> m_pBloomGaussianWeights;

		std::unique_ptr<ComputeKernel> m_pBloomPrefilterKernel;
		std::unique_ptr<ComputeKernel> m_pBloomHorizontalBlurKernel;
		std::unique_ptr<ComputeKernel> m_pBloomVerticalBlurKernel;
		std::unique_ptr<ComputeKernel> m_pBloomCombineKernel;

		std::shared_ptr<RenderTexture> m_pBloomTarget0;
		std::shared_ptr<RenderTexture> m_pBloomTarget1;

		const UINT BloomBlurWidth = 15u;
	};
}