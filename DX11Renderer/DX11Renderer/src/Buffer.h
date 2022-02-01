#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <wrl.h>
#include "DX11Include.h"
#include "BaseBufferData.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace gfx
{
	class Graphics;

	class Buffer : public Bindable
	{
	public:
		Buffer(D3D11_USAGE usage, UINT bindFlags, UINT byteWidth)
			: usage(usage), bindFlags(bindFlags), byteWidth(byteWidth)
		{}
		virtual ~Buffer() {}

	public:
		ComPtr<ID3D11Buffer> GetD3DBuffer()
		{
			return pBuffer;
		}
		ComPtr<ID3D11ShaderResourceView> GetSRV()
		{
			return pSRV;
		}
		ComPtr<ID3D11UnorderedAccessView> GetUAV()
		{
			return pUAV;
		}
	protected:
		ComPtr<ID3D11Buffer> pBuffer;
		ComPtr<ID3D11ShaderResourceView> pSRV;
		ComPtr<ID3D11UnorderedAccessView> pUAV;
		UINT bindFlags;
		D3D11_USAGE usage;
		UINT byteWidth;
	};
}