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
	class ConstantBuffer;
	class StructuredBuffer;

	struct DepthOfFieldCB;

	class DepthOfFieldPass : public RenderPass
	{
	public:
		enum class DepthOfFieldBokehType : u8
		{
			DiskBokeh,
			HexBokeh,
		};
	private:
		enum class DepthOfFieldSubpass : u8
		{
			PrefilterSubpass,
			FarBlurSubpass,
			NearBlurSubpass,
			CompositeSubpass,
		};
	public:
		DepthOfFieldPass(const GraphicsDevice& gfx, const DepthOfFieldBokehType bokehType);
	public:
		void SetOutputTarget(std::shared_ptr<Texture> pTarget);
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const RenderTexture& pDownsampledColor, const RenderTexture& pHiZBufferTarget, const RenderTexture& pCameraColor);
		void DrawImguiControls(const GraphicsDevice& gfx) override;
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const override;
	private:
		void ExecuteDiskBokeh(const GraphicsDevice& gfx, RenderState& renderState) const;
		void ExecuteHexBokeh(const GraphicsDevice& gfx, RenderState& renderState) const;
	private:
		DepthOfFieldBokehType m_bokehType;

		float m_focusDistance = 38.23f;
		float m_focusWidth = 7.2f;
		float m_farFadeWidth = 19.61f;
		float m_nearFadeWidth = 7.69f;
		float m_farIntensity = 1.0f;
		float m_nearIntensity = 1.0f;

		// Shared
		std::unique_ptr<ComputeKernel> m_pDoFPrefilterKernel;

		// Disk bokeh
		std::unique_ptr<ComputeKernel> m_pDoFHorizontalFilterKernel;
		std::unique_ptr<ComputeKernel> m_pDoFVerticalFilterAndCombineKernel;
		std::unique_ptr<ComputeKernel> m_pDoFCompositeKernel;

		// Hex bokeh
		std::unique_ptr<ComputeKernel> m_pVerticalHexFilterKernel;
		std::unique_ptr<ComputeKernel> m_pDiagonalHexFilterKernel;
		std::unique_ptr<ComputeKernel> m_pRhomboidHexFilterKernel;
		//std::unique_ptr<ComputeKernel> m_pDoFVerticalFilterAndCombineKernel;
		//std::unique_ptr<ComputeKernel> m_pDoFCompositeKernel;

		std::unique_ptr<StructuredBuffer> m_pBokehDiskWeights;
		std::unique_ptr<ConstantBuffer> m_pDepthOfFieldCB;
		std::unique_ptr<DepthOfFieldCB> m_depthOfFieldCB;

		std::shared_ptr<RenderTexture> m_pDoFFar0;
		std::shared_ptr<RenderTexture> m_pDoFFar1;
		std::shared_ptr<RenderTexture> m_pDoFFar2;
		std::shared_ptr<RenderTexture> m_pDoFFar3;
		std::shared_ptr<RenderTexture> m_pDoFNear0;
		std::shared_ptr<RenderTexture> m_pDoFNear1;
		std::shared_ptr<RenderTexture> m_pDoFNear2;
		std::shared_ptr<RenderTexture> m_pDoFNear3;

		std::unique_ptr<Sampler> m_pClampedMaxSampler;

		const UINT BokehDiskWidth = 31u;
		const UINT BokehDiskComponentElements = BokehDiskWidth * 2u + 1u;
	};
}