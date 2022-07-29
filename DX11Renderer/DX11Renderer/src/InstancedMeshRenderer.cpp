#include "pch.h"
#include "InstancedMeshRenderer.h"
#include "GraphicsDevice.h"
#include "Material.h"
#include "VertexBufferWrapper.h"
#include "IndexBuffer.h"
#include "Topology.h"
#include <exception>
#include <assert.h>
#include "DepthStencilState.h"
#include "MeshAsset.h"

namespace gfx
{
	InstancedMeshRenderer::InstancedMeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::vector<std::shared_ptr<VertexBufferWrapper>> _pVertexBuffers, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, const UINT instanceCount)
		: MeshRenderer(gfx, name, pMeshAsset, pMaterial, _pVertexBuffers, pIndexBuffer, pTopologyBuffer),
		m_instanceCount(instanceCount)
	{

	}

	void InstancedMeshRenderer::IssueDrawCall(const GraphicsDevice& gfx) const
	{
		gfx.DrawIndexedInstanced(GetIndexCount(), m_instanceCount);
	}
}