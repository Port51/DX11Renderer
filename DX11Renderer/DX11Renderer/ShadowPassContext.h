#pragma once

//class ShadowPassContext {};

#include <memory>
#include "Graphics.h"
#include "Camera.h"
#include "Renderer.h"
#include "RendererList.h"
#include "CommonCbuffers.h"
#include "ConstantBuffer.h"

class ShadowPassContext
{
public:
	/*ShadowPassContext(Graphics& gfx, const Camera& cam, Renderer& renderer, const std::unique_ptr<RenderPass>& pRenderPass, std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB, std::shared_ptr<RendererList>& pRendererList)
		: gfx(gfx), cam(cam), renderer(renderer), pRenderPass(pRenderPass), pTransformationCB(pTransformationCB), pRendererList(pRendererList)
	{}
	Graphics& gfx;
	const Camera& cam;
	Renderer& renderer;
	const std::unique_ptr<RenderPass>& pRenderPass;
	std::shared_ptr<RendererList>& pRendererList;
	std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB;*/
};