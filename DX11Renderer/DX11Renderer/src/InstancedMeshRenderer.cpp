#include "pch.h"
#include "InstancedMeshRenderer.h"
#include "Material.h"
#include "VertexBufferWrapper.h"
#include "IndexBuffer.h"
#include "Topology.h"
#include <exception>
#include <assert.h>
#include "DepthStencilState.h"

namespace gfx
{
	InstancedMeshRenderer::InstancedMeshRenderer(GraphicsDevice & gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> _pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount)
		: MeshRenderer(gfx, name, pMaterial, _pVertexBuffer, pIndexBuffer, pTopologyBuffer),
		instanceCount(instanceCount)
	{

	}

	void InstancedMeshRenderer::IssueDrawCall(GraphicsDevice& gfx) const
	{
		gfx.DrawIndexedInstanced(GetIndexCount(), instanceCount);
	}
}