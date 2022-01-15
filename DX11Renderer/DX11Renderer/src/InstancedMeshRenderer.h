#pragma once
#include "MeshRenderer.h"
#include <wrl.h>
#include <vector>
#include <string>

class Graphics;
class Material;
class VertexBufferWrapper;
class IndexBuffer;
class Topology;

class InstancedMeshRenderer : public MeshRenderer
{
public:
	InstancedMeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount);
public:
	void IssueDrawCall(Graphics& gfx) const override;
private:
	UINT instanceCount;
};