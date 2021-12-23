#pragma once
#include "Material.h"
#include <d3d11.h>
#include <string>
#include <memory>

class Graphics;
class VertexBufferWrapper;
class IndexBuffer;
class Topology;
class Renderer;
class InputLayout;
class TransformCbuf;

struct DrawContext;
struct Transforms;

class MeshRenderer
{
public:
	MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
	MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
	MeshRenderer(const MeshRenderer& other)
	{
		throw std::runtime_error("Copy constructor called!");
	};
	MeshRenderer& operator=(const MeshRenderer& other)
	{
		throw std::runtime_error("Copy assignment operator called!");
		return *this;
	};
public:
	dx::XMMATRIX GetTransformXM() const;
	void SetTransform(dx::XMMATRIX transform);
	void SubmitDrawCalls(const DrawContext& drawContext) const;
	virtual void Bind(Graphics& gfx, const DrawContext& drawContext) const;
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