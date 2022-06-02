#include "pch.h"
#include "RawBuffer.h"
#include "GraphicsDevice.h"

namespace gfx
{
	RawBuffer::RawBuffer(const GraphicsDevice& gfx, const UINT bytes, const D3D11_USAGE usage, const UINT bindFlags)
		: Buffer(usage, bindFlags, 1u)
	{
		// Refs:
		//	- https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-intro#raw-views-of-buffers
		//	- https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-rwbyteaddressbuffer

		// Must be in chunks of 32 bits

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = usage;
		bd.ByteWidth = bytes;
		bd.BindFlags = bindFlags;
		bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

		THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&bd, nullptr, &m_pBuffer));

		if (bindFlags& D3D11_BIND_SHADER_RESOURCE)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
			srvd.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			
			THROW_IF_FAILED(gfx.GetAdapter()->CreateShaderResourceView(m_pBuffer.Get(), &srvd, &m_pSRV));
		}

		if (bindFlags& D3D11_BIND_UNORDERED_ACCESS)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = bytes;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			THROW_IF_FAILED(gfx.GetAdapter()->CreateUnorderedAccessView(m_pBuffer.Get(), &uavDesc, &m_pUAV));
		}
	}
}