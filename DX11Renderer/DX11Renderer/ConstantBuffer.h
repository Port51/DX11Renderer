#pragma once
#include "Buffer.h"
#include <d3d11.h>

class Graphics;

template<typename C>
class ConstantBuffer : public Buffer
{
public:
	ConstantBuffer(Graphics& gfx)
		: Buffer(gfx, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0u)
	{}
public:
	void BindCS(Graphics& gfx, UINT slot) override
	{
		gfx.pContext->CSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindVS(Graphics& gfx, UINT slot) override
	{
		gfx.pContext->VSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindPS(Graphics& gfx, UINT slot) override
	{
		gfx.pContext->PSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void Update(Graphics& gfx, const std::vector<C> data)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_MAPPED_SUBRESOURCE subresource;
		// Map() locks resource and gives ptr to resource
		GFX_THROW_NOINFO(gfx.pContext->Map(
			pBuffer.Get(), 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&subresource // msr gets assigned to resource ptr
		));
		// Handle write
		memcpy(subresource.pData, data.data(), sizeof(C) * data.size());
		// Unlock via Unmap()
		gfx.pContext->Unmap(pBuffer.Get(), 0u);
	}
};
