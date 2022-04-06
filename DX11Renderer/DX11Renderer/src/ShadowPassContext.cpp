#include "pch.h"
#include "ShadowPassContext.h"
#include "Camera.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "RendererList.h"
#include "CommonCbuffers.h"
#include "ConstantBuffer.h"

gfx::ShadowPassContext::ShadowPassContext(GraphicsDevice& gfx, const std::unique_ptr<Camera>& cam, Renderer & renderer, const std::unique_ptr<RenderPass>& pRenderPass, std::unique_ptr<ConstantBuffer<GlobalTransformCB>>& pTransformationCB, std::shared_ptr<RendererList> pRendererList)
	: gfx(gfx), pCamera(cam), renderer(renderer), pRenderPass(pRenderPass), pTransformationCB(pTransformationCB), pRendererList(pRendererList)
{
	Update();
}

void gfx::ShadowPassContext::Update()
{
	invViewMatrix = dx::XMMatrixInverse(nullptr, pCamera->GetViewMatrix());
}
