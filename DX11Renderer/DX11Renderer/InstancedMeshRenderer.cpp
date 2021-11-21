#include "InstancedMeshRenderer.h"
#include <d3d11.h>

InstancedMeshRenderer::InstancedMeshRenderer(Graphics & gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount)
	: MeshRenderer(gfx, name, pMaterial, pVertexBuffer, pIndexBuffer, pTopologyBuffer),
	instanceCount(instanceCount)
{
	struct InstanceData
	{
		DirectX::XMFLOAT3 positionWS;
	};

	InstanceData* instances = new InstanceData[instanceCount];
	for (int i = 0; i < instanceCount; ++i)
	{
		instances[i].positionWS = DirectX::XMFLOAT3(i, 0, 0);
	}

	// Create our trees instance buffer
	// Pretty much the same thing as a regular vertex buffer, except that this buffers data
	// will be used per "instance" instead of per "vertex". Each instance of the geometry
	// gets it's own instanceData data, similar to how each vertex of the geometry gets its own
	// Vertex data
	D3D11_BUFFER_DESC instBuffDesc;
	ZeroMemory(&instBuffDesc, sizeof(instBuffDesc));

	instBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	instBuffDesc.ByteWidth = sizeof(InstanceData) * instanceCount;
	instBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instBuffDesc.CPUAccessFlags = 0;
	instBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA instData;
	ZeroMemory(&instData, sizeof(instData));

	instData.pSysMem = &instances[0];
	gfx.pDevice->CreateBuffer(&instBuffDesc, &instData, &pInstanceBuffer);

	// Release the instance array now that the instance buffer has been created and loaded.
	delete[] instances;
}

void InstancedMeshRenderer::Bind(Graphics& gfx) const
{
	MeshRenderer::Bind(gfx);
}

void InstancedMeshRenderer::IssueDrawCall(Graphics& gfx) const
{
	gfx.pContext->DrawIndexedInstanced(GetIndexCount(), instanceCount, 0u, 0, 0u);
}
