#pragma once
#include "Buffer.h"
#include "RenderPass.h"
#include "DX11Include.h"

class Graphics;

template<typename T>
class ConstantBuffer : public Buffer
{
public:
	ConstantBuffer(Graphics& gfx, D3D11_USAGE usage)
		: Buffer(usage, D3D11_BIND_CONSTANT_BUFFER, GetCBufferSize(sizeof(T)))
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = usage;
		bd.ByteWidth = GetCBufferSize(sizeof(T));
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		bd.StructureByteStride = 0u;

		THROW_IF_FAILED(gfx.GetDevice()->CreateBuffer(&bd, nullptr, &pBuffer));
	}
	ConstantBuffer(Graphics& gfx, D3D11_USAGE usage, const T& initialData)
		: Buffer(usage, D3D11_BIND_CONSTANT_BUFFER, GetCBufferSize(sizeof(T)))
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = usage;
		bd.ByteWidth = GetCBufferSize(sizeof(T));
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		bd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = (void*)&initialData;
		THROW_IF_FAILED(gfx.GetDevice()->CreateBuffer(&bd, &sd, &pBuffer));
	}
public:
	void BindCS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->CSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindVS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->VSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindPS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->PSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void Update(Graphics& gfx, const T& data)
	{
		Update(gfx, &data, sizeof(T));
	}
	void Update(Graphics& gfx, const void* data, UINT dataSize)
	{
		if (usage == D3D11_USAGE_DYNAMIC) // Can be continuously modified by CPU
		{
			D3D11_MAPPED_SUBRESOURCE subresource;
			// Map() locks resource and gives ptr to resource
			THROW_IF_FAILED(gfx.GetContext()->Map(
				pBuffer.Get(), 0u,
				D3D11_MAP_WRITE_DISCARD, 0u,
				&subresource // msr gets assigned to resource ptr
			));
			// Handle write
			memcpy(subresource.pData, data, dataSize);
			// Unlock via Unmap()
			gfx.GetContext()->Unmap(pBuffer.Get(), 0u);
		}
		else if (usage == D3D11_USAGE_DEFAULT
			|| usage == D3D11_USAGE_STAGING)
		{
			// Doesn't work for dynamic or immutable
			gfx.GetContext()->UpdateSubresource(pBuffer.Get(), 0, nullptr, &data, 0, 0);
		}
		else
		{
			throw std::runtime_error("Cannot update immutable constant buffer!");
		}
	}
private:
	static constexpr UINT GetCBufferSize(UINT buffer_size)
	{
		return (buffer_size + (64 - 1)) & ~(64 - 1);
	}
};
