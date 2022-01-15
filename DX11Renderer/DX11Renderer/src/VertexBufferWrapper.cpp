#include "pch.h"
#include "VertexBufferWrapper.h"
#include "BaseBufferData.h"
#include "SharedCodex.h"
#include "Graphics.h"

VertexBufferWrapper::VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer)
{
	strides.resize(1);
	offsets.resize(1);
	pBufferArray.resize(1);
	SetupVertexBuffer(gfx, vertexBuffer);
}

VertexBufferWrapper::VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer)
{
	strides.resize(2);
	offsets.resize(2);
	pBufferArray.resize(2);
	SetupVertexBuffer(gfx, vertexBuffer);
	SetupInstanceBuffer(gfx, instanceBuffer);
}

ComPtr<ID3D11Buffer> VertexBufferWrapper::GetVertexBuffer() const
{
	return pBufferArray[0];
}

void VertexBufferWrapper::BindIA(Graphics& gfx, UINT slot)
{
	gfx.GetContext()->IASetVertexBuffers(slot, pBufferArray.size(), pBufferArray[0].GetAddressOf(), &strides[0], &offsets[0]);
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

	strides[0] = data.GetStride();
	offsets[0] = 0;

	assert(data.GetStride() % 16 == 0 && "Vertex buffer stride must be a multiple of 16");

	D3D11_SUBRESOURCE_DATA sd = data.GetSubresourceData();
	gfx.GetDevice()->CreateBuffer(&bd, &sd, &pBufferArray[0]);

	vertexCount = data.GetElementCount();
}

void VertexBufferWrapper::SetupInstanceBuffer(Graphics& gfx, const BaseBufferData& data)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = UINT(data.GetSizeInBytes());
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = data.GetStride();

	strides[1] = data.GetStride();
	offsets[1] = 0;

	assert(data.GetStride() % 16 == 0 && "Instance buffer stride must be a multiple of 16");

	D3D11_SUBRESOURCE_DATA sd = data.GetSubresourceData();
	gfx.GetDevice()->CreateBuffer(&bd, &sd, &pBufferArray[1]);
}

UINT VertexBufferWrapper::GetVertexCount() const
{
	return vertexCount;
}
