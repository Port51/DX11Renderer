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

	struct GlobalTransformCB;
	struct PerFrameCB;
	struct PerCameraCB;
	struct HiZCreationCB;
	struct ClusteredLightingCB;
	struct FXAA_CB;
	struct SSR_CB;
	struct DitherCB;

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	class Renderer
	{
	public:
		enum RendererView { Final, TiledLighting, ClusteredLighting, SSRTrace };
		enum RendererFeature { Shadows, FXAA, HZBSSR, Dither, Tonemapping, COUNT }; // for COUNT to be accurate, don't set these values to anything weird...
	public:
		Renderer(const GraphicsDevice& gfx, std::shared_ptr<LightManager> pLightManager, std::shared_ptr<RendererList> pRendererList);
		virtual ~Renderer();
		virtual void Release();
	public:
		void AcceptDrawCall(DrawCall job, std::string targetPass);
		void Execute(GraphicsDevice& gfx, const Camera& cam, float timeElapsed, UINT pixelSelectionX, UINT pixelSelectionY);
		void DrawImguiControlWindow(const GraphicsDevice& gfx);
		void Reset();
		bool IsFeatureEnabled(RendererFeature feature) const;
	private:
		void SetupRenderPassDependencies(const GraphicsDevice& gfx);
		RenderPass& GetRenderPass(const std::string name) const;
		const RenderPass& CreateRenderPass(const std::string name);
		const RenderPass& CreateRenderPass(const std::string name, std::unique_ptr<RenderPass> pRenderPass);

	private:
		std::unordered_map<std::string, std::unique_ptr<RenderPass>> m_pRenderPasses;

		std::shared_ptr<Sampler> m_pClampedBilinearSampler;

		std::shared_ptr<RenderTexture> m_pNormalRoughReflectivityTarget;
		std::shared_ptr<RenderTexture> m_pSpecularLighting;
		std::shared_ptr<RenderTexture> m_pDiffuseLighting;
		std::shared_ptr<RenderTexture> m_pHiZBufferTarget;
		std::shared_ptr<RenderTexture> m_pBloomPyramid;

		// Debug views
		std::shared_ptr<RenderTexture> m_pDebugTiledLighting;
		std::shared_ptr<RenderTexture> m_pDebugClusteredLighting;
		std::shared_ptr<RenderTexture> m_pDebugSSR;

		std::shared_ptr<RenderTexture> m_pCameraColor0;
		std::shared_ptr<RenderTexture> m_pCameraColor1;
		bool m_pFinalBlitInputIsIndex0;

		std::unique_ptr<ComputeKernel> m_pHiZDepthCopyKernel;
		std::unique_ptr<ComputeKernel> m_pHiZCreateMipKernel;
		std::unique_ptr<ComputeKernel> m_pTiledLightingKernel;
		std::unique_ptr<ComputeKernel> m_pClusteredLightingKernel;
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

	public:
		const std::string PerCameraPassName = std::string("PerCameraPass");
		const std::string ShadowPassName = std::string("ShadowPass");
		const std::string DepthPrepassName = std::string("DepthPrepass");
		const std::string HiZPassName = std::string("HiZPass");
		const std::string GBufferRenderPassName = std::string("GBuffer");
		const std::string TiledLightingPassName = std::string("TiledLighting");
		const std::string ClusteredLightingPassName = std::string("ClusteredLighting");
		const std::string OpaqueRenderPassName = std::string("Geometry");
		const std::string BlurPyramidPassName = std::string("BlurPyramid");
		const std::string SSRRenderPassName = std::string("SSR");
		const std::string FXAARenderPassName = std::string("FXAA");
		const std::string DitherRenderPassName = std::string("Dither");
		const std::string TonemappingRenderPassName = std::string("Tonemapping");
		const std::string FinalBlitRenderPassName = std::string("FinalBlit");
	};
}