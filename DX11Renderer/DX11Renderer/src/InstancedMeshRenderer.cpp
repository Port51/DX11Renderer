#include "pch.h"
#include "InstancedMeshRenderer.h"
#include "BindableInclude.h"
#include <exception>
#include <assert.h>
#include "DepthStencilState.h"

InstancedMeshRenderer::InstancedMeshRenderer(Graphics & gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> _pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount)
	: MeshRenderer(gfx, name, pMaterial, _pVertexBuffer, pIndexBuffer, pTopologyBuffer),
	instanceCount(instanceCount)
{
	
}

void InstancedMeshRenderer::IssueDrawCall(Graphics& gfx) const
{
	gfx.GetContext()->DrawIndexedInstanced(GetIndexCount(), instanceCount, 0u, 0, 0u);
}
