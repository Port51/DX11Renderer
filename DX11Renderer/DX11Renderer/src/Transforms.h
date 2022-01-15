#pragma once
#include "DXMathInclude.h"

struct Transforms
{
	dx::XMMATRIX model;
	dx::XMMATRIX modelView;
	dx::XMMATRIX modelViewProj;
};