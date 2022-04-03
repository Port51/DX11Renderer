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
	class GraphicsDevice;

	class Buffer : public Bindable
	{
	public:
		Buffer(D3D11_USAGE usage, UINT bindFlags, UINT byteWidth);
		virtual ~Buffer();

	public:
		ComPtr<ID3D11Buffer> GetD3DBuffer();
		ComPtr<ID3D11ShaderResourceView> GetSRV();
		ComPtr<ID3D11UnorderedAccessView> GetUAV();
	protected:
		ComPtr<ID3D11Buffer> m_pBuffer;
		ComPtr<ID3D11ShaderResourceView> m_pSRV;
		ComPtr<ID3D11UnorderedAccessView> m_pUAV;
		UINT m_bindFlags;
		D3D11_USAGE m_usage;
		UINT m_byteWidth;
	};
}