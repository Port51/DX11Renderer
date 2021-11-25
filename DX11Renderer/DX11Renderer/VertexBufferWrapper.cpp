#include "VertexBufferWrapper.h"
#include "BaseBufferData.h"
#include "BindableCodex.h"

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

Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBufferWrapper::GetVertexBuffer() const
{
	return pBufferArray[0];
}

void VertexBufferWrapper::BindIA(Graphics& gfx, UINT slot)
{
	/*static ID3D11Buffer* temp[2];
	temp[0] = pBufferArray[0].Get();
	temp[1] = pBufferArray[1].Get();

	static UINT s[2];
	s[0] = strides[0];
	s[1] = strides[1];

	static UINT o[2];
	o[0] = offsets[0];
	o[1] = offsets[1];*/

	gfx.pContext->IASetVertexBuffers(slot, pBufferArray.size(), pBufferArray[0].GetAddressOf(), &strides[0], &offsets[0]);
	//gfx.pContext->IASetVertexBuffers(slot, pBufferArray.size(), temp, s, o);
	//delete[] temp;
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
	gfx.pDevice->CreateBuffer(&bd, &sd, &pBufferArray[0]);

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
	gfx.pDevice->CreateBuffer(&bd, &sd, &pBufferArray[1]);
}

size_t VertexBufferWrapper::GetVertexCount() const
{
	return vertexCount;
}
