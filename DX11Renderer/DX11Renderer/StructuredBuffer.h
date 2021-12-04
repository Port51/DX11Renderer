#pragma once
#include "Buffer.h"
#include <d3d11.h>

class Graphics;

template<typename C>
class StructuredBuffer : public Buffer
{
public:
	StructuredBuffer(Graphics& gfx, D3D11_USAGE usage, UINT bindFlags, size_t numElements, bool useCounter = false)
		: Buffer(usage, bindFlags, sizeof(C)),
		useCounter(useCounter)
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
			gfx.GetDevice()->CreateShaderResourceView(pBuffer.Get(), nullptr, &pSRV);
		}

		if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			if (!useCounter)
			{
				gfx.GetDevice()->CreateUnorderedAccessView(pBuffer.Get(), nullptr, &pUAV);
			}
			else
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = numElements;
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				gfx.GetDevice()->CreateUnorderedAccessView(pBuffer.Get(), &uavDesc, &pUAV);
			}
		}
	}

	void BindCS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->CSSetShaderResources(slot, 1u, pSRV.GetAddressOf());
	}
	void BindVS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->VSSetShaderResources(slot, 1u, pSRV.GetAddressOf());
	}
	void BindPS(Graphics& gfx, UINT slot) override
	{
		gfx.GetContext()->PSSetShaderResources(slot, 1u, pSRV.GetAddressOf());
	}
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV;
	bool useCounter;
};
