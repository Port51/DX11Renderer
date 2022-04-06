#include "pch.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "VertexBufferWrapper.h"
#include "IndexBuffer.h"
#include "Topology.h"
#include <exception>
#include <assert.h>
#include "DepthStencilState.h"
#include "RenderConstants.h"
#include "DrawContext.h"
#include "Transforms.h"
#include "TransformCbuf.h"

namespace gfx
{
	MeshRenderer::MeshRenderer(GraphicsDevice& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
		: m_pIndexBuffer(std::move(pIndexBuffer)),
		m_pTopology(std::move(pTopologyBuffer)),
		m_name(name),
		m_pMaterial(pMaterial)
	{
		assert("Material cannot be null" && pMaterial);
		gfx.GetLog()->Info("Create MeshRenderer " + name);

		m_pTransformCbuf = std::make_shared<TransformCbuf>(gfx);
	}

	MeshRenderer::MeshRenderer(GraphicsDevice& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
		: MeshRenderer(gfx, name, pMaterial, pIndexBuffer, pTopologyBuffer)
	{
		m_pVertexBufferWrapper = std::move(pVertexBuffer);
	}

	const dx::XMMATRIX MeshRenderer::GetTransformXM() const
	{
		return dx::XMLoadFloat4x4(&m_transform);
	}

	void MeshRenderer::SetTransform(dx::XMMATRIX _transform)
	{
		dx::XMStoreFloat4x4(&m_transform, _transform);
	}

	void MeshRenderer::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		m_pMaterial->SubmitDrawCalls(*this, drawContext);
	}

	void MeshRenderer::Bind(GraphicsDevice& gfx, const DrawContext& drawContext) const
	{
		m_pTopology->BindIA(gfx, 0u);
		m_pIndexBuffer->BindIA(gfx, 0u);
		m_pVertexBufferWrapper->BindIA(gfx, 0u);

		const auto modelMatrix = GetTransformXM();
		const auto modelViewMatrix = modelMatrix * drawContext.viewMatrix;
		const auto modelViewProjectMatrix = modelViewMatrix * drawContext.projMatrix;
		Transforms transforms{ modelMatrix, modelViewMatrix, modelViewProjectMatrix };
		m_pTransformCbuf->UpdateTransforms(gfx, transforms);

		m_pTransformCbuf->BindVS(gfx, RenderSlots::VS_TransformCB);
	}

	const UINT MeshRenderer::GetIndexCount() const
	{
		return m_pIndexBuffer->GetIndexCount();
	}

	const UINT MeshRenderer::GetVertexCount() const
	{
		return m_pVertexBufferWrapper->GetVertexCount();
	}

	const AABB& MeshRenderer::GetAABB() const
	{
		return m_aabb;
	}

	void MeshRenderer::IssueDrawCall(GraphicsDevice& gfx) const
	{
		gfx.DrawIndexed(GetIndexCount());
	}
}