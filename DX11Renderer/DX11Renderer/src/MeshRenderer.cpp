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
#include "MeshAsset.h"

namespace gfx
{
	MeshRenderer::MeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
		: m_pMeshAsset(std::move(pMeshAsset)),
		m_pIndexBuffer(std::move(pIndexBuffer)),
		m_pTopology(std::move(pTopologyBuffer)),
		m_name(name),
		m_pMaterial(std::move(pMaterial))
	{
		assert("Material cannot be null" && m_pMaterial != nullptr);
		gfx.GetLog().Info("Create MeshRenderer " + m_name);

		m_pTransformCbuf = std::make_shared<TransformCbuf>(gfx);
	}

	MeshRenderer::MeshRenderer(const GraphicsDevice& gfx, const std::string name, std::shared_ptr<MeshAsset> pMeshAsset, std::shared_ptr<Material> pMaterial, std::vector<std::shared_ptr<VertexBufferWrapper>> pVertexBuffers, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
		: MeshRenderer(gfx, name, std::move(pMeshAsset), std::move(pMaterial), std::move(pIndexBuffer), std::move(pTopologyBuffer))
	{
		m_pVertexBufferWrappers = std::move(pVertexBuffers);
	}

	const dx::XMMATRIX MeshRenderer::GetTransformXM() const
	{
		return dx::XMLoadFloat4x4(&m_transform);
	}

	void MeshRenderer::SetTransform(const dx::XMMATRIX _transform)
	{
		dx::XMStoreFloat4x4(&m_transform, _transform);
	}

	void MeshRenderer::SubmitDrawCommands(const DrawContext& drawContext) const
	{
		m_pMaterial->SubmitDrawCommands(*this, drawContext);
	}

	void MeshRenderer::Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const
	{
		const auto attributesIdx = m_pMaterial->GetAttributesIndex(drawContext.renderPass);

		m_pTopology->BindIA(gfx, renderState, 0u);
		m_pIndexBuffer->BindIA(gfx, renderState, 0u);
		m_pVertexBufferWrappers.at(attributesIdx)->BindIA(gfx, renderState, 0u);

		// todo: is there a way to only update this for the main camera once?
		const auto modelMatrix = GetTransformXM();
		const auto modelViewMatrix = modelMatrix * drawContext.viewMatrix;
		const auto modelViewProjectMatrix = modelViewMatrix * drawContext.projMatrix;
		const Transforms transforms{ modelMatrix, modelViewMatrix, modelViewProjectMatrix };
		m_pTransformCbuf->UpdateTransforms(gfx, transforms);

		m_pTransformCbuf->BindVS(gfx, renderState, RenderSlots::VS_TransformCB);
	}

	const UINT MeshRenderer::GetIndexCount() const
	{
		return m_indexCount;
	}

	const UINT MeshRenderer::GetVertexCount() const
	{
		return m_vertexCount;
	}

	const AABB& MeshRenderer::GetAABB() const
	{
		return m_pMeshAsset->m_aabb;
	}

	const u64 MeshRenderer::GetMaterialCode() const
	{
		return m_pMaterial->GetMaterialCode();
	}

	void MeshRenderer::IssueDrawCall(const GraphicsDevice& gfx) const
	{
		gfx.DrawIndexed(GetIndexCount());
	}
}