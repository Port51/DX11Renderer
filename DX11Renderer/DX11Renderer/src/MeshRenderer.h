#pragma once
#include "Material.h"
#include "CommonHeader.h"
#include "DX11Include.h"
#include <string>

namespace gfx
{
	class GraphicsDevice;
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
		MeshRenderer(GraphicsDevice& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
		MeshRenderer(GraphicsDevice& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
		virtual ~MeshRenderer() = default;

	public:
		dx::XMMATRIX GetTransformXM() const;
		void SetTransform(dx::XMMATRIX transform);
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		virtual void Bind(GraphicsDevice& gfx, const DrawContext& drawContext) const;
		UINT GetIndexCount() const;
		UINT GetVertexCount() const;
		virtual void IssueDrawCall(GraphicsDevice& gfx) const;
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
}