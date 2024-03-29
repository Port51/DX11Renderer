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
	InstancedMeshRenderer::InstancedMeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> _pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, const UINT instanceCount)
		: MeshRenderer(gfx, name, pMeshAsset, pMaterial, _pVertexBuffer, pIndexBuffer, pTopologyBuffer),
		m_instanceCount(instanceCount)
	{

	}

	void InstancedMeshRenderer::IssueDrawCall(const GraphicsDevice& gfx) const
	{
		if (m_instanceCount == 0u) THROW("You dummy, you forgot to set instance counts!");
		gfx.DrawIndexedInstanced(GetIndexCount(), m_instanceCount);
	}

	const bool InstancedMeshRenderer::UseModelTransform() const
	{
		return false;
	}
}