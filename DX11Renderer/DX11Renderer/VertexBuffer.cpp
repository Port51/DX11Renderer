#include "VertexBuffer.h"
#include "BindableCodex.h"

VertexBuffer::VertexBuffer(Graphics& gfx, const VertexBufferData& vbuf)
	: VertexBuffer(gfx, "?", vbuf)
{}
VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const VertexBufferData& vbuf)
	: stride(vbuf.GetSizeInBytes()),
	vertexCount(vbuf.GetVertexCount()),
	tag(tag)
{
	SETUP_LOGGING(gfx);

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = UINT(vbuf.GetSizeInBytes());
	bd.StructureByteStride = stride;
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vbuf.GetData();
	GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer));
}

Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer::GetVertexBuffer() const
{
	return pVertexBuffer;
}

void VertexBuffer::BindIA(Graphics& gfx, UINT slot)
{
	const UINT offset = 0u;
	GetContext(gfx)->IASetVertexBuffers(slot, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
}

std::shared_ptr<VertexBuffer> VertexBuffer::Resolve(Graphics& gfx, const std::string& tag, const VertexBufferData& vbuf)
{
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

UINT VertexBuffer::GetVertexCount() const
{
	return vertexCount;
}

UINT VertexBuffer::GetStride() const
{
	return stride;
}
