#include "pch.h"
#include "Buffer.h"

namespace gfx
{
	Buffer::Buffer(D3D11_USAGE usage, UINT bindFlags, UINT byteWidth)
		: m_usage(usage), m_bindFlags(bindFlags), m_byteWidth(byteWidth)
	{}

	Buffer::~Buffer() {}

	ComPtr<ID3D11Buffer> Buffer::GetD3DBuffer()
	{
		return m_pBuffer;
	}

	ComPtr<ID3D11ShaderResourceView> Buffer::GetSRV()
	{
		return m_pSRV;
	}

	ComPtr<ID3D11UnorderedAccessView> Buffer::GetUAV()
	{
		return m_pUAV;
	}
}