#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include <vector>
#include <wrl.h>
#include "DX11Include.h"

struct ID3D11Buffer;

namespace gfx
{
	class GraphicsDevice;

	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(GraphicsDevice& gfx, const std::vector<u16>& indices);
		IndexBuffer(GraphicsDevice& gfx, const std::vector<u32>& indices);
	public:
		void BindIA(GraphicsDevice& gfx, UINT slot) override;
		UINT GetIndexCount() const;
	public:
		static std::shared_ptr<IndexBuffer> Resolve(GraphicsDevice& gfx, std::string id, const std::vector<u32>& indices);
	private:
		static std::string GenerateUID(const std::string& tag);
	protected:
		UINT m_count;
		DXGI_FORMAT m_format;
		ComPtr<ID3D11Buffer> m_pIndexBuffer;
	};
}