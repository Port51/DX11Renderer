#pragma once
#include "Buffer.h"
#include <d3d11.h>

class Graphics;

template<typename C>
class ConstantBuffer : public Buffer<C>
{
public:
	ConstantBuffer(Graphics& gfx)
		: Buffer<C>(gfx, identifier, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0u)
	{}
};
