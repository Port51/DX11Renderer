#pragma once
#include <memory>
#include "DX11Include.h"
#include "Camera.h"

class Graphics;
class Camera;
class Renderer;
class RenderPass;
class RendererList;

struct GlobalTransformCB;

template<typename Type>
class ConstantBuffer;

class ShadowPassContext
{
public:
	ShadowPassContext(Graphics& gfx, const Camera& cam, Renderer& renderer, const std::unique_ptr<RenderPass>& pRenderPass, std::unique_ptr<ConstantBuffer<GlobalTransformCB>>& pTransformationCB, std::shared_ptr<RendererList> pRendererList)
		: gfx(gfx), cam(cam), renderer(renderer), pRenderPass(pRenderPass), pTransformationCB(pTransformationCB), pRendererList(pRendererList)
	{
		invViewMatrix = dx::XMMatrixInverse(nullptr, cam.GetViewMatrix());
	}
	Graphics& gfx;
	const Camera& cam;
	dx::XMMATRIX invViewMatrix;
	Renderer& renderer;
	const std::unique_ptr<RenderPass>& pRenderPass;
	std::unique_ptr<ConstantBuffer<GlobalTransformCB>>& pTransformationCB;
	std::shared_ptr<RendererList> pRendererList;
};