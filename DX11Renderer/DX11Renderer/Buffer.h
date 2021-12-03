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
	Buffer(Graphics& gfx, D3D11_BUFFER_DESC bufferDesc, const BaseBufferData& data)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &data.GetSubresourceData();
		GFX_THROW_NOINFO(gfx.pDevice->CreateBuffer(&bufferDesc, &sd, &pBuffer));
	}

	Buffer(Graphics& gfx, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG cpuAccessFlag, UINT miscFlags, UINT byteWidth)
	{
		SETUP_LOGGING_NOINFO(gfx);

		D3D11_BUFFER_DESC bd;
		bd.BindFlags = bindFlag;
		bd.Usage = usage;
		bd.CPUAccessFlags = cpuAccessFlag;
		bd.MiscFlags = miscFlags;
		bd.ByteWidth = byteWidth;
		bd.StructureByteStride = 0u;

		GFX_THROW_NOINFO(gfx.pDevice->CreateBuffer(&bd, nullptr, &pBuffer));
	}

public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3DBuffer()
	{
		return pBuffer;
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
};
