#include "pch.h"
#include "IndexBuffer.h"
#include "SharedCodex.h"
#include "Graphics.h"

namespace gfx
{
	IndexBuffer::IndexBuffer(Graphics& gfx, const std::vector<unsigned short>& indices)
		: count((UINT)indices.size())
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = UINT(count * sizeof(unsigned short));
		ibd.StructureByteStride = sizeof(unsigned short);
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = indices.data();
		THROW_IF_FAILED(gfx.GetDevice()->CreateBuffer(&ibd, &isd, &pIndexBuffer));
	}

	void IndexBuffer::BindIA(Graphics& gfx, UINT slot)
	{
		gfx.GetContext()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	}

	UINT IndexBuffer::GetIndexCount() const
	{
		return count;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Resolve(Graphics& gfx, std::string id, const std::vector<unsigned short>& indices)
	{
		return Codex::Resolve<IndexBuffer>(gfx, GenerateUID(id), indices);
	}

	std::string IndexBuffer::GenerateUID(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(IndexBuffer).name() + "#"s + tag;
	}
}