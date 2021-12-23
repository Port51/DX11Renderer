#pragma once
#include <memory>

class Graphics;
class Camera;
class Renderer;
class RenderPass;
class RendererList;

struct TransformationCB;

template<typename Type>
class ConstantBuffer;

class ShadowPassContext
{
public:
	ShadowPassContext(Graphics& gfx, const Camera& cam, Renderer& renderer, const std::unique_ptr<RenderPass>& pRenderPass, std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB, std::shared_ptr<RendererList> pRendererList)
		: gfx(gfx), cam(cam), renderer(renderer), pRenderPass(pRenderPass), pTransformationCB(pTransformationCB), pRendererList(pRendererList)
	{}
	Graphics& gfx;
	const Camera& cam;
	Renderer& renderer;
	const std::unique_ptr<RenderPass>& pRenderPass;
	std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB;
	std::shared_ptr<RendererList> pRendererList;
};