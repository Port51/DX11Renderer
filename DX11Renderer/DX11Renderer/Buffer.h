#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"
#include <string>
#include "WindowsInclude.h"
#include <wrl.h>
#include <d3d11.h>

class Graphics;

template<typename C>
class Buffer : public Bindable
{
public:
	Buffer(Graphics& gfx, std::string identifier, D3D11_BUFFER_DESC bufferDesc, const C& data)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &data;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bufferDesc, &sd, &pBuffer));
	}

	// Init with constants
	Buffer(Graphics& gfx, std::string identifier, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags, const C& data)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = bindFlag;
		bd.Usage = usage;
		bd.CPUAccessFlags = cpuAccessFlag;
		bd.MiscFlags = miscFlags;
		bd.ByteWidth = sizeof(data);
		bd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &data;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &csd, &pBuffer));
	}

	Buffer(Graphics& gfx, std::string identifier, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = bindFlag;
		bd.Usage = usage;
		bd.CPUAccessFlags = cpuAccessFlag;
		bd.MiscFlags = miscFlags;
		bd.ByteWidth = sizeof(C);
		bd.StructureByteStride = 0u;

		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, nullptr, &pBuffer));
	}

	// Don't init
	Buffer(Graphics& gfx, std::string identifier)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0u;
		bd.ByteWidth = sizeof(C);
		bd.StructureByteStride = 0u;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, nullptr, &pBuffer));
	}
public:
	void BindCS(Graphics& gfx, UINT slot) override
	{
		GetContext(gfx)->CSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindVS(Graphics& gfx, UINT slot) override
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	void BindPS(Graphics& gfx, UINT slot) override
	{
		GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pBuffer.GetAddressOf());
	}
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3DBuffer()
	{
		return pBuffer;
	}
	void Update(Graphics& gfx, const C& data)
	{
		SETUP_LOGGING(gfx);

		D3D11_MAPPED_SUBRESOURCE subresource;
		// Map() locks resource and gives ptr to resource
		GFX_THROW_INFO(GetContext(gfx)->Map(
			pBuffer.Get(), 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&subresource // msr gets assigned to resource ptr
		));
		// Handle write
		memcpy(subresource.pData, &data, sizeof(data));
		// Unlock via Unmap()
		GetContext(gfx)->Unmap(pBuffer.Get(), 0u);
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
	std::string identifier;
};
