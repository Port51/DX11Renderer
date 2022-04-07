#include "pch.h"
#include "ShadowPassContext.h"
#include "Camera.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "RendererList.h"
#include "CommonCbuffers.h"
#include "ConstantBuffer.h"

namespace gfx
{
	ShadowPassContext::ShadowPassContext(GraphicsDevice& gfx, const Camera& cam, Renderer& renderer, RenderPass& renderPass, ConstantBuffer<GlobalTransformCB>& transformationCB, RendererList* pRendererList)
		: gfx(gfx), camera(cam), renderer(renderer), renderPass(renderPass), transformationCB(transformationCB), pRendererList(pRendererList)
	{
		Update();
	}

	void ShadowPassContext::Update()
	{
		invViewMatrix = dx::XMMatrixInverse(nullptr, camera.GetViewMatrix());
	}
}