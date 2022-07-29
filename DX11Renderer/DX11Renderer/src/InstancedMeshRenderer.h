#pragma once
#include "MeshRenderer.h"
#include <wrl.h>
#include <vector>
#include <string>

namespace gfx
{
	class GraphicsDevice;
	class Material;
	class VertexBufferWrapper;
	class IndexBuffer;
	class Topology;
	class MeshAsset;

	class InstancedMeshRenderer : public MeshRenderer
	{
	public:
		InstancedMeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::vector<std::shared_ptr<VertexBufferWrapper>> pVertexBuffers, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, const UINT instanceCount);
	public:
		void IssueDrawCall(const GraphicsDevice& gfx) const override;
	private:
		UINT m_instanceCount;
	};
}