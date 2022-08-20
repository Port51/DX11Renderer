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
	ShadowPassContext::ShadowPassContext(GraphicsDevice& gfx, const Camera& cam, Renderer& renderer, RenderPass& renderPass, ConstantBuffer& transformationCB, RendererList* pRendererList)
		: gfx(gfx), camera(cam), renderer(renderer), renderPass(renderPass), transformationCB(transformationCB), pRendererList(pRendererList)
	{
	}

	ShadowPassContext::~ShadowPassContext()
	{}
}