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
	class Graphics;

	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(Graphics& gfx, const std::vector<u16>& indices);
		IndexBuffer(Graphics& gfx, const std::vector<u32>& indices);
	public:
		void BindIA(Graphics& gfx, UINT slot) override;
		UINT GetIndexCount() const;
	public:
		static std::shared_ptr<IndexBuffer> Resolve(Graphics& gfx, std::string id, const std::vector<u32>& indices);
	private:
		static std::string GenerateUID(const std::string& tag);
	protected:
		UINT count;
		DXGI_FORMAT format;
		ComPtr<ID3D11Buffer> pIndexBuffer;
	};
}