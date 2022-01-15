#include "pch.h"
#include "MeshRenderer.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include <exception>
#include <assert.h>
#include "DepthStencilState.h"
#include "RenderConstants.h"
#include "DrawContext.h"
#include "Transforms.h"

MeshRenderer::MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
	: pIndexBuffer(std::move(pIndexBuffer)),
	pTopology(std::move(pTopologyBuffer)),
	name(name),
	pMaterial(pMaterial)
{
	assert("Material cannot be null" && pMaterial);
	gfx.GetLog().Info("Create MeshRenderer " + name);

	pTransformCbuf = std::make_shared<TransformCbuf>(gfx);
}

MeshRenderer::MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<VertexBufferWrapper> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
	: MeshRenderer(gfx, name, pMaterial, pIndexBuffer, pTopologyBuffer)
{
	pVertexBufferWrapper = std::move(pVertexBuffer);
}

dx::XMMATRIX MeshRenderer::GetTransformXM() const
{
	return dx::XMLoadFloat4x4(&transform);
}

void MeshRenderer::SetTransform(dx::XMMATRIX _transform)
{
	dx::XMStoreFloat4x4(&transform, _transform);
}

void MeshRenderer::SubmitDrawCalls(const DrawContext& drawContext) const
{
	pMaterial->SubmitDrawCalls(*this, drawContext);
}

void MeshRenderer::Bind(Graphics& gfx, const DrawContext& drawContext) const
{
	pTopology->BindIA(gfx, 0u);
	pIndexBuffer->BindIA(gfx, 0u);
	pVertexBufferWrapper->BindIA(gfx, 0u);
	
	const auto modelMatrix = GetTransformXM();
	const auto modelViewMatrix = modelMatrix * drawContext.viewMatrix;
	const auto modelViewProjectMatrix = modelViewMatrix * drawContext.projMatrix;
	Transforms transforms{ modelMatrix, modelViewMatrix, modelViewProjectMatrix };
	pTransformCbuf->UpdateTransforms(gfx, transforms);

	pTransformCbuf->BindVS(gfx, RenderSlots::VS_TransformCB);
}

UINT MeshRenderer::GetIndexCount() const
{
	return pIndexBuffer->GetIndexCount();
}

UINT MeshRenderer::GetVertexCount() const
{
	return pVertexBufferWrapper->GetVertexCount();
}

void MeshRenderer::IssueDrawCall(Graphics& gfx) const
{
	gfx.DrawIndexed(GetIndexCount());
}
