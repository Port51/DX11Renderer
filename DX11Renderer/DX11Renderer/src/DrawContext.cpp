#include "pch.h"
#include "DrawContext.h"

namespace gfx
{
	DrawContext::DrawContext(Renderer& renderer)
		: renderer(renderer)
	{}

	DrawContext::DrawContext(Renderer& renderer, RenderPassType _renderPass)
		: renderer(renderer), renderPasses(std::vector<RenderPassType> { _renderPass })
	{}

	DrawContext::DrawContext(Renderer& renderer, std::vector<RenderPassType> _renderPasses)
		: renderer(renderer), renderPasses(std::move(_renderPasses))
	{}

	void DrawContext::SetRenderPasses(RenderPassType _renderPass)
	{
		renderPasses.clear();
		renderPasses.emplace_back(_renderPass);
	}

	void DrawContext::SetRenderPasses(std::vector<RenderPassType> _renderPasses)
	{
		renderPasses = std::move(_renderPasses);
	}
}