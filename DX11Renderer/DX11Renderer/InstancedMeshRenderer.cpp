#include "InstancedMeshRenderer.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include <exception>
#include <assert.h>
#include "Stencil.h"

InstancedMeshRenderer::InstancedMeshRenderer(Graphics & gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> _pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount)
	: MeshRenderer(gfx, name, pMaterial, _pVertexBuffer, pIndexBuffer, pTopologyBuffer),
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

	

	// Release the instance array now that the instance buffer has been created and loaded.
	delete[] instances;
}

void InstancedMeshRenderer::IssueDrawCall(Graphics& gfx) const
{
	gfx.pContext->DrawIndexedInstanced(GetIndexCount(), instanceCount, 0u, 0, 0u);
}
