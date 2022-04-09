#include "pch.h"
#include "Buffer.h"

namespace gfx
{
	Buffer::Buffer(D3D11_USAGE usage, UINT bindFlags, UINT byteWidth)
		: m_usage(usage), m_bindFlags(bindFlags), m_byteWidth(byteWidth)
	{}

	Buffer::~Buffer()
	{
		m_pUAV.Reset();
		m_pSRV.Reset();
		m_pBuffer.Reset();
	}

	void Buffer::Release()
	{
		m_pSRV.Reset();
		m_pUAV.Reset();
		m_pBuffer.Reset();
	}

	const ComPtr<ID3D11Buffer> Buffer::GetD3DBuffer() const
	{
		return m_pBuffer;
	}

	const ComPtr<ID3D11ShaderResourceView> Buffer::GetSRV() const
	{
		return m_pSRV;
	}

	const ComPtr<ID3D11UnorderedAccessView> Buffer::GetUAV() const
	{
		return m_pUAV;
	}
}