#pragma once
#include "CommonHeader.h"
#include "ExceptionHandling.h"
#include "Buffer.h"
#include "RenderPass.h"
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	template<typename T>
	class ConstantBuffer : public Buffer
	{
	public:
		ConstantBuffer(const GraphicsDevice& gfx, const D3D11_USAGE usage)
			: Buffer(usage, D3D11_BIND_CONSTANT_BUFFER, GetCBufferSize(sizeof(T)))
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = usage;
			bd.ByteWidth = GetCBufferSize(sizeof(T));
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
			bd.StructureByteStride = 0u;

			THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&bd, nullptr, &m_pBuffer));
		}

		ConstantBuffer(const GraphicsDevice& gfx, const D3D11_USAGE usage, const T& initialData)
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
			THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&bd, &sd, &m_pBuffer));
		}

	public:
		void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override
		{
			if (renderState.IsNewBinding(GetGuid(), RenderBindingType::CS_CB, slot))
			{
				gfx.GetContext()->CSSetConstantBuffers(slot, 1u, m_pBuffer.GetAddressOf());
			}
		}

		void UnbindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override
		{
			renderState.ClearBinding(RenderBindingType::CS_CB, slot);
			gfx.GetContext()->CSSetConstantBuffers(slot, 1u, RenderConstants::NullBufferArray.data());
		}

		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override
		{
			if (renderState.IsNewBinding(GetGuid(), RenderBindingType::VS_CB, slot))
			{
				gfx.GetContext()->VSSetConstantBuffers(slot, 1u, m_pBuffer.GetAddressOf());
			}
		}

		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override
		{
			renderState.ClearBinding(RenderBindingType::VS_CB, slot);
			gfx.GetContext()->VSSetConstantBuffers(slot, 1u, RenderConstants::NullBufferArray.data());
		}

		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override
		{
			if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_CB, slot))
			{
				gfx.GetContext()->PSSetConstantBuffers(slot, 1u, m_pBuffer.GetAddressOf());
			}
		}

		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override
		{
			renderState.ClearBinding(RenderBindingType::PS_CB, slot);
			gfx.GetContext()->PSSetConstantBuffers(slot, 1u, RenderConstants::NullBufferArray.data());
		}

		void Update(const GraphicsDevice& gfx, const T& data)
		{
			Update(gfx, &data, sizeof(T));
		}

		void Update(const GraphicsDevice& gfx, const void* data, const UINT dataSize)
		{
			if (m_usage == D3D11_USAGE_DYNAMIC) // Can be continuously modified by CPU
			{
				D3D11_MAPPED_SUBRESOURCE subresource;
				// Map() locks resource and gives ptr to resource
				THROW_IF_FAILED(gfx.GetContext()->Map(
					m_pBuffer.Get(), 0u,
					D3D11_MAP_WRITE_DISCARD, 0u,
					&subresource // msr gets assigned to resource ptr
				));
				// Handle write
				memcpy(subresource.pData, data, dataSize);
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
				THROW("Cannot update immutable constant buffer!");
			}
		}

	private:
		static constexpr UINT GetCBufferSize(const UINT buffer_size)
		{
			return (buffer_size + (64 - 1))& ~(64 - 1);
		}
	};
}