#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"
#include <string>
#include "WindowsInclude.h"
#include <wrl.h>
#include <d3d11.h>

class Graphics;

template<typename C>
class ConstantBuffer : public Bindable
{
public:
	// Init with constants
	ConstantBuffer(Graphics & gfx, std::string identifier, const C & data)
		: identifier(identifier)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(data);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &data;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer));
	}

	// Don't init
	ConstantBuffer(Graphics & gfx, std::string identifier)
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
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pConstantBuffer));
	}
public:
	void Update(Graphics & gfx, const C & data)
	{
		SETUP_LOGGING(gfx);

		D3D11_MAPPED_SUBRESOURCE subresource;
		// Map() locks resource and gives ptr to resource
		GFX_THROW_INFO(GetContext(gfx)->Map(
			pConstantBuffer.Get(), 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&subresource // msr gets assigned to resource ptr
		));
		// Handle write
		memcpy(subresource.pData, &data, sizeof(data));
		// Unlock via Unmap()
		GetContext(gfx)->Unmap(pConstantBuffer.Get(), 0u);
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	std::string identifier;
};
