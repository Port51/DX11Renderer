#include "MeshRenderer.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include <exception>
#include <assert.h>
#include "DepthStencilState.h"
#include "RenderConstants.h"

MeshRenderer::MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
	: pIndexBuffer(std::move(pIndexBuffer)),
	pTopology(std::move(pTopologyBuffer)),
	name(name),
	pMaterial(pMaterial)
{
	assert("Material cannot be null" && pMaterial);

	pTransformCbuf = std::make_shared<TransformCbuf>(gfx, *this);
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

// Called via Node
void MeshRenderer::SubmitDrawCalls(Renderer& renderer) const
{
	pMaterial->SubmitDrawCalls(renderer, *this);
}

void MeshRenderer::Bind(Graphics& gfx) const
{
	pTopology->BindIA(gfx, 0u);
	pIndexBuffer->BindIA(gfx, 0u);
	pVertexBufferWrapper->BindIA(gfx, 0u);
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
