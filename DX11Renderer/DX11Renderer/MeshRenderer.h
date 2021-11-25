#pragma once
#include "Material.h"
#include <d3d11.h>
#include <string>
#include <memory>

class Graphics;
class VertexBufferWrapper;
class IndexBuffer;
class Topology;
class FrameCommander;
class InputLayout;
class TransformCbuf;

namespace dx = DirectX;

class MeshRenderer
{
public:
	MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
	MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
	dx::XMMATRIX GetTransformXM() const;
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, dx::FXMMATRIX _accumulatedTranform) const;
	virtual void Bind(Graphics& gfx) const;
	UINT GetIndexCount() const;
	UINT GetVertexCount() const;
	virtual void IssueDrawCall(Graphics& gfx) const;
protected:
	std::string name;
	std::shared_ptr<Material> pMaterial; // keep separate from other bindables for now...
	mutable dx::XMFLOAT4X4 transform;
protected:
	std::shared_ptr<IndexBuffer> pIndexBuffer;
	std::shared_ptr<VertexBufferWrapper> pVertexBufferWrapper;
	std::shared_ptr<Topology> pTopology;
	std::shared_ptr<TransformCbuf> pTransformCbuf;
};