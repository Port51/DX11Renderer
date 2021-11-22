#pragma once
#include "Material.h"
#include <d3d11.h>
#include <string>
#include <memory>

class Graphics;
class VertexBuffer;
class IndexBuffer;
class Topology;
class FrameCommander;
class InputLayout;
class TransformCbuf;
//class Technique;

namespace dx = DirectX;

class MeshRenderer
{
public:
	MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
	DirectX::XMMATRIX GetTransformXM() const;
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, dx::FXMMATRIX _accumulatedTranform) const;
	virtual void Bind(Graphics& gfx) const;
	UINT GetIndexCount() const;
	UINT GetVertexCount() const;
	virtual void IssueDrawCall(Graphics& gfx) const;
private:
	std::string name;
	std::shared_ptr<Material> pMaterial; // keep separate from other bindables for now...
	mutable dx::XMFLOAT4X4 transform;
private:
	std::shared_ptr<IndexBuffer> pIndices;
	std::shared_ptr<VertexBuffer> pVertices;
	std::shared_ptr<Topology> pTopology;
	std::shared_ptr<TransformCbuf> pTransformCbuf;
};