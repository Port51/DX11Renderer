#include "VertexBufferWrapper.h"
#include "BaseBufferData.h"
#include "BindableCodex.h"

VertexBufferWrapper::VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vbuf)
	: VertexBufferWrapper(gfx, "?", vbuf)
{}
VertexBufferWrapper::VertexBufferWrapper(Graphics& gfx, const std::string& tag, const BaseBufferData& vbuf)
	: strides(vbuf.GetStride()),
	vertexCount(vbuf.GetElementCount()),
	tag(tag)
{
	pBufferArray.resize(1);
}

Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBufferWrapper::GetVertexBuffer() const
{
	return pBufferArray[0];
}

void VertexBufferWrapper::BindIA(Graphics& gfx, UINT slot)
{
	const UINT offsets = 0u;
	gfx.pContext->IASetVertexBuffers(slot, pBufferArray.size(), pBufferArray[0].GetAddressOf(), &strides, &offsets);
}

void VertexBufferWrapper::SetupVertexBuffer(Graphics& gfx, const BaseBufferData& data)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = UINT(data.GetSizeInBytes());
	bd.StructureByteStride = data.GetStride();

	D3D11_SUBRESOURCE_DATA sd = data.GetSubresourceData();
	gfx.pDevice->CreateBuffer(&bd, &sd, &pBufferArray[0]);
}

void VertexBufferWrapper::SetupInstanceBuffer(Graphics& gfx, const BaseBufferData& data)
{
	// Create our trees instance buffer
	// Pretty much the same thing as a regular vertex buffer, except that this buffers data
	// will be used per "instance" instead of per "vertex". Each instance of the geometry
	// gets it's own instanceData data, similar to how each vertex of the geometry gets its own
	// Vertex data
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = UINT(data.GetSizeInBytes());
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = data.GetStride();

	D3D11_SUBRESOURCE_DATA sd = data.GetSubresourceData();
	gfx.pDevice->CreateBuffer(&bd, &sd, &pBufferArray[1]);
}

UINT VertexBufferWrapper::GetVertexCount() const
{
	return vertexCount;
}

UINT VertexBufferWrapper::GetStride() const
{
	return strides;
}
