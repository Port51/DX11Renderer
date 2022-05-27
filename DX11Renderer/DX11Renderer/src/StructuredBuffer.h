#pragma once
#include "Buffer.h"
#include "DX11Include.h"
#include "GraphicsDevice.h"

namespace gfx
{

	template<typename T>
	class StructuredBuffer : public Buffer
	{
	public:
		StructuredBuffer(const GraphicsDevice& gfx, D3D11_USAGE usage, UINT bindFlags, UINT numElements, const void* initialData, bool useCounter = false)
			: Buffer(usage, bindFlags, sizeof(T)),
			m_numElements(numElements),
			m_useCounter(useCounter)
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = usage;
			bd.ByteWidth = sizeof(T) * numElements;
			bd.BindFlags = bindFlags;
			bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0u;
			bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bd.StructureByteStride = sizeof(T);

			if (initialData != nullptr)
			{
				D3D11_SUBRESOURCE_DATA srd;
				srd.pSysMem = initialData;
				srd.SysMemPitch = 0u;
				srd.SysMemSlicePitch = 0u;

				THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&bd, &srd, &m_pBuffer));
			}
			else
			{
				THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&bd, nullptr, &m_pBuffer));
			}

			if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(m_pBuffer.Get(), nullptr, &m_pSRV));
			}

			if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				if (!useCounter)
				{
					THROW_IF_FAILED(gfx.GetAdapter()->CreateUnorderedAccessView(m_pBuffer.Get(), nullptr, &m_pUAV));
				}
				else
				{
					D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					uavDesc.Buffer.FirstElement = 0;
					uavDesc.Buffer.NumElements = numElements;
					uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					THROW_IF_FAILED(gfx.GetAdapter()->CreateUnorderedAccessView(m_pBuffer.Get(), &uavDesc, &m_pUAV));
				}
			}
		}

		StructuredBuffer(const GraphicsDevice& gfx, D3D11_USAGE usage, UINT bindFlags, UINT numElements, bool useCounter = false)
			: StructuredBuffer(gfx, usage, bindFlags, numElements, nullptr, useCounter)
		{}

		void BindCS(const GraphicsDevice& gfx, UINT slot) override
		{
			gfx.GetContext()->CSSetShaderResources(slot, 1u, m_pSRV.GetAddressOf());
		}

		void BindVS(const GraphicsDevice& gfx, UINT slot) override
		{
			gfx.GetContext()->VSSetShaderResources(slot, 1u, m_pSRV.GetAddressOf());
		}

		void BindPS(const GraphicsDevice& gfx, UINT slot) override
		{
			gfx.GetContext()->PSSetShaderResources(slot, 1u, m_pSRV.GetAddressOf());
		}

		void Update(const GraphicsDevice& gfx, const void* data, UINT dataBytes)
		{
			if (m_usage == D3D11_USAGE_DYNAMIC) // Can be continuously modified by CPU
			{
				D3D11_MAPPED_SUBRESOURCE subresource = {};
				// Map() locks resource and gives ptr to resource
				THROW_IF_FAILED(gfx.GetContext()->Map(
					m_pBuffer.Get(), 0u,
					D3D11_MAP_WRITE_DISCARD, 0u,
					&subresource // msr gets assigned to resource ptr
				));
				// Handle write
				memcpy(subresource.pData, data, dataBytes);
				// Unlock via Unmap()
				gfx.GetContext()->Unmap(m_pBuffer.Get(), 0u);
			}
			else if (m_usage == D3D11_USAGE_DEFAULT
				|| m_usage == D3D11_USAGE_STAGING)
			{
				// Doesn't work for dynamic or immutable
				gfx.GetContext()->UpdateSubresource(m_pBuffer.Get(), 0, nullptr, &data, 0, 0);
			}
			else
			{
				throw std::runtime_error("Cannot update immutable structured buffer!");
			}
		}

		void Update(const GraphicsDevice& gfx, const std::vector<T>& data, UINT dataElements)
		{
			assert(dataElements <= m_numElements && "Data elements exceed buffer size!");
			Update(gfx, data.data(), sizeof(T) * dataElements);
		}
	private:
		bool m_useCounter;
		UINT m_numElements;
	};
}