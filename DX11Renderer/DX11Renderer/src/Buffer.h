#pragma once
#include "Bindable.h"
#include "Common.h"
#include "GraphicsThrowMacros.h"
#include <wrl.h>
#include "DX11Include.h"
#include "BaseBufferData.h"

class Graphics;

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

class Buffer : public Bindable
{
public:
	Buffer(D3D11_USAGE usage, UINT bindFlags, UINT byteWidth)
		: usage(usage), bindFlags(bindFlags), byteWidth(byteWidth)
	{}
	virtual ~Buffer() {}

public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3DBuffer()
	{
		return pBuffer;
	}
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetD3DSRV()
	{
		return pSRV;
	}
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> GetD3DUAV()
	{
		return pUAV;
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV;
	UINT bindFlags;
	D3D11_USAGE usage;
	UINT byteWidth;
};
