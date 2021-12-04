#pragma once
#include "Buffer.h"
#include <d3d11.h>

class Graphics;

template<typename C>
class StructuredBuffer : public Buffer
{
public:
	StructuredBuffer(Graphics& gfx, D3D11_USAGE usage, UINT bindFlags, size_t numElements)
		: Buffer(usage, bindFlags, sizeof(C))
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = usage;
		bd.ByteWidth = sizeof(C) * numElements;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		bd.StructureByteStride = 0u;

		GFX_THROW_NOINFO(gfx.GetDevice()->CreateBuffer(&bd, nullptr, &pBuffer));

		if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			device->CreateShaderResourceView(buffer.Get(), nullptr, &pSRV);
		}

		if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			if (!counter)
			{
				gfx.GetDevice()->CreateUnorderedAccessView(buffer.Get(), nullptr, &pUAV);
			}
			else
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = numElements;
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				gfx.GetDevice()->CreateUnorderedAccessView(buffer.Get(), &uavDesc, &pUAV);
				has_counter = true;
			}
		}
	}

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
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV;
};
