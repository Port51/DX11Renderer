#include "pch.h"
#include "DrawContext.h"

namespace gfx
{
	DrawContext::DrawContext(Renderer & renderer)
		: renderer(renderer)
	{}
	DrawContext::DrawContext(Renderer & renderer, std::string _renderPass)
		: renderer(renderer), renderPasses(std::vector<std::string> { _renderPass })
	{}
	DrawContext::DrawContext(Renderer & renderer, std::vector<std::string> _renderPasses)
		: renderer(renderer), renderPasses(std::move(_renderPasses))
	{}
	void DrawContext::SetRenderPasses(std::string _renderPass)
	{
		renderPasses.clear();
		renderPasses.emplace_back(_renderPass);
	}
	void DrawContext::SetRenderPasses(std::vector<std::string> _renderPasses)
	{
		renderPasses = std::move(_renderPasses);
	}
}