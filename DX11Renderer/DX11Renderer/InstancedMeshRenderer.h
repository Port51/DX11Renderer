#pragma once
#include "MeshRenderer.h"
#include "WindowsInclude.h"

class Graphics;
class Material;
class VertexBuffer;
class IndexBuffer;
class Topology;

class InstancedMeshRenderer : public MeshRenderer
{
public:
	InstancedMeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer, UINT instanceCount);
public:
	void Bind(Graphics& gfx) const override;
	void IssueDrawCall(Graphics& gfx) const override;
private:
	UINT instanceCount;
};