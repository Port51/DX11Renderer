#pragma once

#include "CommonHeader.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace gfx
{
	class GraphicsDevice;
	class Camera;
	class LightManager;
	class RendererList;
	class DrawCall;
	class RenderPass;
	class DepthStencilTarget;
	class RenderTexture;
	class ComputeKernel;
	class Sampler;
	class Texture;
	class RandomGenerator;

	struct GlobalTransformCB;
	struct PerFrameCB;
	struct PerCameraCB;
	struct HiZCreationCB;
	struct ClusteredLightingCB;
	struct FXAA_CB;
	struct SSR_CB;
	struct DitherCB;
	struct DepthOfFieldCB;
	struct BloomCB;

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	enum RenderPassType : int;

	class Renderer
	{
	public:
		enum RendererView { Final, TiledLighting, ClusteredLighting, SSRTrace };
		enum RendererFeature { Shadows, DepthOfField, Bloom, FXAA, SSAO, HZBSSR, Dither, Tonemapping, COUNT }; // for COUNT to be accurate, don't set these values to anything weird...
		
	public:
		Renderer(const GraphicsDevice& gfx, RandomGenerator& rng, std::shared_ptr<LightManager> pLightManager, std::shared_ptr<RendererList> pRendererList);
		virtual ~Renderer();
		virtual void Release();
	public:
		void AcceptDrawCall(DrawCall job, RenderPassType targetPass);
		void Execute(GraphicsDevice& gfx, const Camera& cam, float timeElapsed, UINT pixelSelectionX, UINT pixelSelectionY);
		void DrawImguiControlWindow(const GraphicsDevice& gfx);
		void Reset();
		bool IsFeatureEnabled(RendererFeature feature) const;
	private:
		void SetupRenderPassDependencies(const GraphicsDevice& gfx);
		RenderPass& GetRenderPass(const RenderPassType pass) const;
		const RenderPass& CreateRenderPass(const RenderPassType pass);
		const RenderPass& CreateRenderPass(const RenderPassType pass, std::unique_ptr<RenderPass> pRenderPass);

	private:
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> m_pRenderPasses;

		std::shared_ptr<Sampler> m_pClampedBilinearSampler;

		std::shared_ptr<RenderTexture> m_pNormalRoughReflectivityTarget;
		std::shared_ptr<RenderTexture> m_pSpecularLighting;
		std::shared_ptr<RenderTexture> m_pDiffuseLighting;
		std::shared_ptr<RenderTexture> m_pHiZBufferTarget;

		// Debug views
		std::shared_ptr<RenderTexture> m_pDebugTiledLighting;
		std::shared_ptr<RenderTexture> m_pDebugClusteredLighting;
		std::shared_ptr<RenderTexture> m_pDebugSSR;

		std::shared_ptr<RenderTexture> m_pCameraColor0;
		std::shared_ptr<RenderTexture> m_pCameraColor1;
		std::shared_ptr<RenderTexture> m_pDownsampledColor;
		bool m_pFinalBlitInputIsIndex0;

		std::unique_ptr<ComputeKernel> m_pHiZDepthCopyKernel;
		std::unique_ptr<ComputeKernel> m_pHiZCreateMipKernel;
		std::unique_ptr<ComputeKernel> m_pTiledLightingKernel;
		std::unique_ptr<ComputeKernel> m_pClusteredLightingKernel;
		std::unique_ptr<ComputeKernel> m_pBilinearDownsampleKernel;
		std::unique_ptr<ComputeKernel> m_pSSRKernel;
		std::unique_ptr<ComputeKernel> m_pFXAAKernel;
		std::unique_ptr<ComputeKernel> m_pDitherKernel;
		std::unique_ptr<ComputeKernel> m_pTonemappingKernel;

		std::unique_ptr<ConstantBuffer<GlobalTransformCB>> m_pTransformationCB;
		std::unique_ptr<ConstantBuffer<PerFrameCB>> m_pPerFrameCB;
		std::unique_ptr<ConstantBuffer<PerCameraCB>> m_pPerCameraCB;
		std::unique_ptr<ConstantBuffer<HiZCreationCB>> m_pHiZCreationCB;
		std::unique_ptr<ConstantBuffer<ClusteredLightingCB>> m_pClusteredLightingCB;
		std::unique_ptr<ConstantBuffer<FXAA_CB>> m_pFXAA_CB;
		std::unique_ptr<ConstantBuffer<SSR_CB>> m_pSSR_CB;
		std::unique_ptr<StructuredBuffer<int>> m_pSSR_DebugData;
		std::unique_ptr<ConstantBuffer<DitherCB>> m_pDitherCB;

		std::shared_ptr<Texture> m_pDitherTexture;

		std::shared_ptr<RendererList> m_pRendererList;
		std::unique_ptr<RendererList> m_pVisibleRendererList; // filtered by camera frustum

		std::shared_ptr<LightManager> m_pLightManager;

		std::shared_ptr<Sampler> m_pShadowSampler;

	private:
		RendererView m_viewIdx = RendererView::Final;
		std::vector<bool> m_rendererFeatureEnabled;
		int m_pixelIteration;
	};
}