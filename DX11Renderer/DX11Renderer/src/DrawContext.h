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
		DrawContext(Renderer& renderer, RenderPassType renderPass);
		DrawContext(Renderer& renderer, RenderPassType renderPass, const dx::XMMATRIX viewMatrix, const dx::XMMATRIX projMatrix);
	public:
		const RenderPassType renderPass;
		Renderer& renderer;
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projMatrix;
	};
}