#pragma once
#include "DXMathInclude.h"
#include "RenderConstants.h"
#include <vector>

namespace gfx
{
	class Renderer;

	struct DrawContext
	{
	public:
		DrawContext(Renderer& renderer);
		DrawContext(Renderer& renderer, RenderPassType _renderPass);
		DrawContext(Renderer& renderer, std::vector<RenderPassType> _renderPasses);
		void SetRenderPasses(RenderPassType _renderPass);
		void SetRenderPasses(std::vector<RenderPassType> _renderPasses);
	public:
		Renderer& renderer;
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projMatrix;
		std::vector<RenderPassType> renderPasses;
	};
}