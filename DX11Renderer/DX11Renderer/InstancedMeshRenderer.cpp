#include "InstancedMeshRenderer.h"

InstancedMeshRenderer::InstancedMeshRenderer(Graphics & gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount)
	: MeshRenderer(gfx, name, pMaterial, pVertexBuffer, pIndexBuffer, pTopologyBuffer),
	instanceCount(instanceCount)
{
}

void InstancedMeshRenderer::Bind(Graphics& gfx) const
{
	MeshRenderer::Bind(gfx);
}

void InstancedMeshRenderer::IssueDrawCall(Graphics& gfx) const
{
	gfx.pContext->DrawIndexedInstanced(GetIndexCount(), instanceCount, 0u, 0, 0u);
}
