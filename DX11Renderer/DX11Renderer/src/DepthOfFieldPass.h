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

	struct DepthOfFieldCB;

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	class DepthOfFieldPass : public RenderPass
	{
	public:
		DepthOfFieldPass(const GraphicsDevice& gfx);
	public:
		void Execute(const GraphicsDevice& gfx) const override;
		void SetOutputTarget(std::shared_ptr<Texture> pTarget);
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const RenderTexture* const pDownsampledColor, const RenderTexture* const pHiZBufferTarget, const RenderTexture* const pCameraColor);
	private:

		std::unique_ptr<ComputeKernel> m_pDoFPrefilterKernel;
		std::unique_ptr<ComputeKernel> m_pDoFHorizontalFilterKernel;
		std::unique_ptr<ComputeKernel> m_pDoFVerticalFilterAndCombineKernel;
		std::unique_ptr<ComputeKernel> m_pDoFCompositeKernel;

		std::unique_ptr<StructuredBuffer<float>> m_pBokehDiskWeights;
		std::unique_ptr<ConstantBuffer<DepthOfFieldCB>> m_pDepthOfFieldCB;

		std::shared_ptr<RenderTexture> m_pDoFFar0;
		std::shared_ptr<RenderTexture> m_pDoFFar1;
		std::shared_ptr<RenderTexture> m_pDoFFar2;
		std::shared_ptr<RenderTexture> m_pDoFFar3;
		std::shared_ptr<RenderTexture> m_pDoFNear0;
		std::shared_ptr<RenderTexture> m_pDoFNear1;
		std::shared_ptr<RenderTexture> m_pDoFNear2;

		const UINT BokehDiskWidth = 31u;
		const UINT BokehDiskComponentElements = BokehDiskWidth * 2u + 1u;
	};
}