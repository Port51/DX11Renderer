#pragma once
#include "Buffer.h"
#include <d3d11.h>

class Graphics;

template<typename C>
class RawBuffer : public Buffer<C>
{
public:
	RawBuffer(Graphics& gfx, std::string identifier)
		: Buffer<C>(gfx, identifier, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0u)
	{}

	RawBuffer(Graphics& gfx, std::string identifier, const C& data)
		: Buffer<C>(gfx, identifier, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0u, data)
	{}

	void BindCS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->CSSetShaderResources(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindVS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->VSSetShaderResources(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindPS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->PSSetShaderResources(slot, 1u, pBuffer.GetAddressOf());
	}
};
