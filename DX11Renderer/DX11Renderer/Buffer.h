#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"
#include <string>
#include "WindowsInclude.h"
#include <wrl.h>
#include <d3d11.h>
#include "BaseBufferData.h"

class Graphics;

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
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
	UINT bindFlags;
	D3D11_USAGE usage;
	UINT byteWidth;
};
