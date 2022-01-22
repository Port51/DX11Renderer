#pragma once
#include "DXMathInclude.h"
#include <vector>

namespace gfx
{
	class Renderer;

	struct DrawContext
	{
	public:
		DrawContext(Renderer& renderer);
		DrawContext(Renderer& renderer, std::string _renderPass);
		DrawContext(Renderer& renderer, std::vector<std::string> _renderPasses);
		void SetRenderPasses(std::string _renderPass);
		void SetRenderPasses(std::vector<std::string> _renderPasses);
	public:
		Renderer& renderer;
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projMatrix;
		std::vector<std::string> renderPasses;
	};
}