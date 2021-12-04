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
	Buffer(UINT bindFlags, D3D11_USAGE usage, UINT byteWidth)
		: bindFlags(bindFlags), usage(usage), byteWidth(byteWidth)
	{}
	/*Buffer(Graphics& gfx, D3D11_BUFFER_DESC bufferDesc, const BaseBufferData& data)
		: bindFlags(bufferDesc.BindFlags), usage(bufferDesc.Usage), byteWidth(bufferDesc.ByteWidth)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &data.GetSubresourceData();
		GFX_THROW_NOINFO(gfx.GetDevice()->CreateBuffer(&bufferDesc, &sd, &pBuffer));
	}

	Buffer(Graphics& gfx, UINT bindFlags, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags, UINT byteWidth)
		: bindFlags(bindFlags), usage(usage), byteWidth(byteWidth)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = bindFlags;
		bd.Usage = usage;
		bd.CPUAccessFlags = cpuAccessFlag;
		bd.MiscFlags = miscFlags;
		bd.ByteWidth = byteWidth;
		bd.StructureByteStride = 0u;

		GFX_THROW_NOINFO(gfx.GetDevice()->CreateBuffer(&bd, nullptr, &pBuffer));
	}*/

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
