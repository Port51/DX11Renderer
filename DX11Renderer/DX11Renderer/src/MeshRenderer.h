#pragma once
#include "Drawable.h"
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
	class MeshAsset;
	class Material;
	class AABB;

	struct DrawContext;
	struct ObjectTransformsCB;

	class MeshRenderer : public Drawable
	{
		friend class AABB;
	public:
		MeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
		MeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
		virtual ~MeshRenderer();

	public:
		const dx::XMMATRIX GetTransformXM() const;
		void SetTransform(const dx::XMMATRIX transform);
		void SubmitDrawCommands(GraphicsDevice& gfx, const DrawContext& drawContext) const;

	public:
		virtual void Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const;
		virtual void IssueDrawCall(const GraphicsDevice& gfx) const;

	protected:
		const virtual bool UseModelTransform() const;

	public:
		const UINT GetIndexCount() const;
		const UINT GetVertexCount() const;
		const AABB& GetAABB() const;
		const u64 GetMaterialCode(const RenderPassType renderPassType) const;
	protected:
		const std::string m_name;
		std::shared_ptr<Material> m_pMaterial; // keep separate from other bindables for now...
		mutable dx::XMFLOAT4X4 m_transform;
	protected:
		std::shared_ptr<MeshAsset> m_pMeshAsset;
		std::shared_ptr<IndexBuffer> m_pIndexBuffer;
		std::shared_ptr<VertexBufferWrapper> m_pVertexBufferWrapper;
		std::shared_ptr<Topology> m_pTopology;
		std::shared_ptr<TransformCbuf> m_pTransformCbuf;
	};
}