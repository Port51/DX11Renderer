#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"
#include <string>
#include "WindowsInclude.h"
#include <wrl.h>
#include <d3d11.h>
#include "BaseBufferData.h"

class Graphics;

class Buffer
{
public:
	Buffer(Graphics& gfx, D3D11_BUFFER_DESC bufferDesc, const BaseBufferData& data)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &data.GetSubresourceData();
		GFX_THROW_NOINFO(gfx.pDevice->CreateBuffer(&bufferDesc, &sd, &pBuffer));
	}

	// Init with constants
	Buffer(Graphics& gfx, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags, const C& data)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = bindFlag;
		bd.Usage = usage;
		bd.CPUAccessFlags = cpuAccessFlag;
		bd.MiscFlags = miscFlags;
		bd.ByteWidth = sizeof(data);
		bd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &data;
		GFX_THROW_NOINFO(gfx.pDevice->CreateBuffer(&bd, &csd, &pBuffer));
	}

	Buffer(Graphics& gfx, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags, UINT byteWidth)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = bindFlag;
		bd.Usage = usage;
		bd.CPUAccessFlags = cpuAccessFlag;
		bd.MiscFlags = miscFlags;
		bd.ByteWidth = byteWidth;
		bd.StructureByteStride = 0u;

		GFX_THROW_NOINFO(gfx.pDevice->CreateBuffer(&bd, nullptr, &pBuffer));
	}

	// Don't init
	Buffer(Graphics& gfx)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0u;
		bd.ByteWidth = sizeof(C);
		bd.StructureByteStride = 0u;
		GFX_THROW_NOINFO(gfx.pDevice->CreateBuffer(&bd, nullptr, &pBuffer));
	}
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3DBuffer()
	{
		return pBuffer;
	}
	void SetData(Graphics& gfx, const BaseBufferData& data)
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
		memcpy(subresource.pData, data.GetSubresourceData().pSysMem, sizeof(data.GetSizeInBytes()));
		// Unlock via Unmap()
		gfx.pContext->Unmap(pBuffer.Get(), 0u);
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
};
