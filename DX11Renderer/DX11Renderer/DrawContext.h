#pragma once
#include "DXMathInclude.h"

struct DrawContext
{
public:
	dx::XMMATRIX viewMatrix;
	dx::XMMATRIX projMatrix;
};