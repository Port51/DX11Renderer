#pragma once
#include "DXMathInclude.h"

class Renderer;

struct DrawContext
{
public:
	DrawContext(Renderer& renderer)
		: renderer(renderer)
	{}
	Renderer& renderer;
	dx::XMMATRIX viewMatrix;
	dx::XMMATRIX projMatrix;
};