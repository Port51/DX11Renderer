#include "VertexBuffer.h"
#include "BindableCodex.h"

namespace Bind
{
	VertexBuffer::VertexBuffer(Graphics& gfx, const VertexBufferData& vbuf)
		: VertexBuffer(gfx, "?", vbuf)
	{}
	VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const VertexBufferData& vbuf)
		: stride((UINT)vbuf.GetLayout().Size()),
		tag(tag)
	{
		SETUP_LOGGING(gfx);

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(vbuf.SizeBytes());
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vbuf.GetData();
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer));
	}

	void VertexBuffer::Bind(Graphics& gfx, UINT slot)
	{
		const UINT offset = 0u;
		GetContext(gfx)->IASetVertexBuffers(slot, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Resolve(Graphics& gfx, const std::string& tag, const VertexBufferData& vbuf)
	{
		assert(tag != "?");
		return Bind::Codex::Resolve<VertexBuffer>(gfx, tag, vbuf);
	}

	std::string VertexBuffer::GenerateNontemplatedUID(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(VertexBuffer).name() + "#"s + tag;
	}

	std::string VertexBuffer::GetUID() const
	{
		return GenerateUID(tag);
	}
}