#include "pch.h"
#include "DrawContext.h"

namespace gfx
{
	DrawContext::DrawContext(Renderer& renderer, RenderPassType renderPass)
		: renderer(renderer), renderPass(renderPass)
	{}

	DrawContext::DrawContext(Renderer& renderer, RenderPassType renderPass, const dx::XMMATRIX viewMatrix, const dx::XMMATRIX projMatrix)
		: renderer(renderer), renderPass(renderPass), viewMatrix(viewMatrix), projMatrix(projMatrix)
	{}
}