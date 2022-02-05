#pragma once
#include "Buffer.h"
#include "DX11Include.h"
#include "Graphics.h"

namespace gfx
{

	template<typename T>
	class StructuredBuffer : public Buffer
	{
	public:
		StructuredBuffer(Graphics& gfx, D3D11_USAGE usage, UINT bindFlags, UINT numElements, bool useCounter = false)
			: Buffer(usage, bindFlags, sizeof(T)),
			useCounter(useCounter)
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = usage;
			bd.ByteWidth = sizeof(T) * numElements;
			bd.BindFlags = bindFlags;
			bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0u;
			bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bd.StructureByteStride = sizeof(T);

			THROW_IF_FAILED(gfx.GetDevice()->CreateBuffer(&bd, nullptr, &pBuffer));

			if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				THROW_IF_FAILED(gfx.GetDevice()->CreateShaderResourceView(pBuffer.Get(), nullptr, &pSRV));
			}

			if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				if (!useCounter)
				{
					THROW_IF_FAILED(gfx.GetDevice()->CreateUnorderedAccessView(pBuffer.Get(), nullptr, &pUAV));
				}
				else
				{
					D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					uavDesc.Buffer.FirstElement = 0;
					uavDesc.Buffer.NumElements = numElements;
					uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					THROW_IF_FAILED(gfx.GetDevice()->CreateUnorderedAccessView(pBuffer.Get(), &uavDesc, &pUAV));
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
		void Update(Graphics& gfx, const void* data, UINT dataBytes)
		{
			if (usage == D3D11_USAGE_DYNAMIC) // Can be continuously modified by CPU
			{
				D3D11_MAPPED_SUBRESOURCE subresource = {};
				// Map() locks resource and gives ptr to resource
				THROW_IF_FAILED(gfx.GetContext()->Map(
					pBuffer.Get(), 0u,
					D3D11_MAP_WRITE_DISCARD, 0u,
					&subresource // msr gets assigned to resource ptr
				));
				// Handle write
				memcpy(subresource.pData, data, dataBytes);
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
				throw std::runtime_error("Cannot update immutable structured buffer!");
			}
		}
		void Update(Graphics& gfx, const std::vector<T>& data, UINT dataElements)
		{
			Update(gfx, data.data(), sizeof(T) * dataElements);
		}
	private:
		bool useCounter;
	};
}