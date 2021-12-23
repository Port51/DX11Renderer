#pragma once
#include <memory>
/*#include "Graphics.h"
#include "Camera.h"
#include "Renderer.h"
#include "RendererList.h"
#include "CommonCbuffers.h"
#include "ConstantBuffer.h"*/

class Graphics;
class Camera;
class Renderer;

//Graphics & gfx, const Camera & cam, const std::unique_ptr<RenderPass>& pass, const std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB

class ShadowPassContext
{
public:
	ShadowPassContext(Graphics& gfx, const Camera& cam, Renderer& renderer)
		: gfx(gfx), cam(cam), renderer(renderer)
	{}
	Graphics& gfx;
	const Camera& cam;
	Renderer& renderer;

	/*ShadowPassContext(Graphics& gfx, const Camera& cam, Renderer& renderer, const std::unique_ptr<RenderPass>& pRenderPass, std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB, std::shared_ptr<RendererList>& pRendererList)
		: gfx(gfx), cam(cam), renderer(renderer), pRenderPass(pRenderPass), pTransformationCB(pTransformationCB), pRendererList(pRendererList)
	{}
	const std::unique_ptr<RenderPass>& pRenderPass;
	std::shared_ptr<RendererList>& pRendererList;
	std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB;*/
};