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

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &data;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bufferDesc, &csd, &pBuffer));
	}

	// Init with constants
	Buffer(Graphics& gfx, std::string identifier, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags, const C& data)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = bindFlag;
		cbd.Usage = usage;
		cbd.CPUAccessFlags = cpuAccessFlag;
		cbd.MiscFlags = miscFlags;
		cbd.ByteWidth = sizeof(data);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &data;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pBuffer));
	}

	Buffer(Graphics& gfx, std::string identifier, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = bindFlag;
		cbd.Usage = usage;
		cbd.CPUAccessFlags = cpuAccessFlag;
		cbd.MiscFlags = miscFlags;
		cbd.ByteWidth = sizeof(C);
		cbd.StructureByteStride = 0u;

		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pBuffer));
	}

	// Don't init
	Buffer(Graphics& gfx, std::string identifier)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(C);
		cbd.StructureByteStride = 0u;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pBuffer));
	}
public:
	void SetupCSBinding(UINT slot)
	{
		bindToComputeStage = slot;
	}
	void SetupVSBinding(UINT slot)
	{
		bindToVertexStage = slot;
	}
	void SetupPSBinding(UINT slot)
	{
		bindToPixelStage = slot;
	}
	void Bind(Graphics& gfx, UINT slot) override
	{
		if (bindToComputeStage != -1)
		{
			GetContext(gfx)->CSSetConstantBuffers((UINT)bindToComputeStage, 1u, pBuffer.GetAddressOf());
		}
		if (bindToVertexStage != -1)
		{
			GetContext(gfx)->VSSetConstantBuffers((UINT)bindToVertexStage, 1u, pBuffer.GetAddressOf());
		}
		if (bindToPixelStage != -1)
		{
			GetContext(gfx)->PSSetConstantBuffers((UINT)bindToPixelStage, 1u, pBuffer.GetAddressOf());
		}
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
	int bindToComputeStage = -1;
	int bindToVertexStage = -1;
	int bindToPixelStage = -1;
};
