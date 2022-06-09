#include "pch.h"
#include "ArgsBuffer.h"
#include "GraphicsDevice.h"

namespace gfx
{
	ArgsBuffer::ArgsBuffer(const GraphicsDevice & gfx, const UINT setsOfArgs, const bool isIndexed)
		: Buffer(D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, setsOfArgs * (isIndexed ? 5u : 4u))
	{
		const size_t elementsPerArgSet = (isIndexed ? 5u : 4u);

		D3D11_BUFFER_DESC bd;
		ZERO_MEM(bd);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(UINT) * elementsPerArgSet * setsOfArgs;
		bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

		THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&bd, nullptr, &m_pBuffer));

		if (bd.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = elementsPerArgSet * setsOfArgs;
			uavDesc.Buffer.Flags = 0;
			uavDesc.Format = DXGI_FORMAT_R32_UINT;
			THROW_IF_FAILED(gfx.GetAdapter()->CreateUnorderedAccessView(m_pBuffer.Get(), &uavDesc, &m_pUAV));
		}
	}
}