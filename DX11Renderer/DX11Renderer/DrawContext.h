#pragma once
#include "DXMathInclude.h"
#include <set>

class Renderer;

struct DrawContext
{
public:
	DrawContext(Renderer& renderer)
		: renderer(renderer)
	{}
	void SetRenderPasses(std::vector<std::string> renderPasses)
	{
		for (const auto& p : renderPasses)
		{
			renderPassesSet.insert(p);
		}
	}
public:
	Renderer& renderer;
	dx::XMMATRIX viewMatrix;
	dx::XMMATRIX projMatrix;
	std::set<std::string> renderPassesSet;
};