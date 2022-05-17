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

	struct DepthOfFieldCB;

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	class BloomPass : public RenderPass
	{
	public:
		BloomPass(const GraphicsDevice& gfx);

	public:
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const RenderTexture& pDownsampledColor, const RenderTexture& pCameraColor);
		void Execute(const GraphicsDevice& gfx) const override;

	private:
		std::unique_ptr<ConstantBuffer<BloomCB>> m_pBloomCB;
		std::unique_ptr<StructuredBuffer<float>> m_pBloomGaussianWeights;

		std::unique_ptr<ComputeKernel> m_pBloomPrefilterKernel;
		std::unique_ptr<ComputeKernel> m_pBloomHorizontalBlurKernel;
		std::unique_ptr<ComputeKernel> m_pBloomVerticalBlurKernel;
		std::unique_ptr<ComputeKernel> m_pBloomCombineKernel;

		std::shared_ptr<RenderTexture> m_pBloomTarget0;
		std::shared_ptr<RenderTexture> m_pBloomTarget1;

		const UINT BloomBlurWidth = 15u;
	};
}