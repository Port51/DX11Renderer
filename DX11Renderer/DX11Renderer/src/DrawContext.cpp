#include "pch.h"
#include "DrawContext.h"

namespace gfx
{
	DrawContext::DrawContext(Renderer & renderer)
		: renderer(renderer)
	{}
	void DrawContext::SetRenderPasses(std::string _renderPass)
	{
		renderPasses.emplace_back(_renderPass);
	}
	void DrawContext::SetRenderPasses(std::vector<std::string> _renderPasses)
	{
		renderPasses = std::move(_renderPasses);
	}
}