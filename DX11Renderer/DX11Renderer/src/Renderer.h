#pragma once

#include "CommonHeader.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace gfx
{
	class Graphics;
	class Camera;
	class LightManager;
	class RendererList;
	class DrawCall;
	class RenderPass;
	class DepthStencilTarget;
	class RenderTarget;
	class ComputeKernel;
	class Sampler;

	struct GlobalTransformCB;
	struct PerFrameCB;
	struct PerCameraCB;

	template<typename Type>
	class ConstantBuffer;

	class Renderer
	{
	public:
		Renderer(Graphics& gfx, const std::unique_ptr<LightManager>& pLightManager, std::shared_ptr<RendererList> pRendererList);
		virtual ~Renderer();
	public:
		void AcceptDrawCall(DrawCall job, std::string targetPass);
		void Execute(Graphics& gfx, const std::unique_ptr<Camera>& cam, const std::unique_ptr<LightManager>& pLightManager);
		void Reset();
	private:
		const std::unique_ptr<RenderPass>& CreateRenderPass(const std::string name);
		const std::unique_ptr<RenderPass>& CreateRenderPass(const std::string name, std::unique_ptr<RenderPass> pRenderPass);

	private:
		std::shared_ptr<DepthStencilTarget> pSmallDepthStencil;
		std::shared_ptr<RenderTarget> pNormalRoughTarget;
		std::unordered_map<std::string, std::unique_ptr<RenderPass>> pRenderPasses;

		std::shared_ptr<RenderTarget> pSpecularLighting;
		std::shared_ptr<RenderTarget> pDiffuseLighting;

		// Debug views
		std::shared_ptr<RenderTarget> pDebugTiledLightingCS;

		std::shared_ptr<RenderTarget> pCameraColor;

		std::unique_ptr<ComputeKernel> pTiledLightingKernel;
		std::unique_ptr<ConstantBuffer<GlobalTransformCB>> pTransformationCB;
		std::unique_ptr<ConstantBuffer<PerFrameCB>> pPerFrameCB;
		std::unique_ptr<ConstantBuffer<PerCameraCB>> pPerCameraCB;

		std::shared_ptr<RendererList> pRendererList;
		std::unique_ptr<RendererList> pVisibleRendererList; // filtered by camera frustum

		std::shared_ptr<Sampler> pShadowSampler;
	public:
		const std::string PerCameraPassName = std::string("PerCameraPass");
		const std::string ShadowPassName = std::string("ShadowPass");
		const std::string DepthPrepassName = std::string("DepthPrepass");
		const std::string GBufferRenderPassName = std::string("GBuffer");
		const std::string TiledLightingPassName = std::string("TiledLighting");
		const std::string GeometryRenderPassName = std::string("Geometry");
		const std::string FinalBlitRenderPassName = std::string("FinalBlit");
	};
}