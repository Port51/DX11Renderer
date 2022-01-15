#pragma once
#include "DXMathInclude.h"
#include <vector>

class Renderer;

struct DrawContext
{
public:
	DrawContext(Renderer& renderer)
		: renderer(renderer)
	{}
	void SetRenderPasses(std::string _renderPass)
	{
		renderPasses.emplace_back(_renderPass);
	}
	void SetRenderPasses(std::vector<std::string> _renderPasses)
	{
		renderPasses = std::move(_renderPasses);
	}
public:
	Renderer& renderer;
	dx::XMMATRIX viewMatrix;
	dx::XMMATRIX projMatrix;
	std::vector<std::string> renderPasses;
};