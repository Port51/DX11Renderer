#include "MeshRenderer.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "VertexInclude.h"
#include <exception>
#include <assert.h>
#include "Stencil.h"

namespace dx = DirectX;

MeshRenderer::MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial,
	std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
	: pVertexBuffer(std::move(pVertexBuffer)),
	pIndexBuffer(std::move(pIndexBuffer)),
	pTopology(std::move(pTopologyBuffer)),
	name(name),
	pMaterial(pMaterial)
{
	assert("Material cannot be null" && pMaterial);

	/*if (!hasIndexBuffer)
	{
		throw std::runtime_error(std::string("Mesh '") + name + std::string("' is missing IndexBuffer!"));
	}*/

	pTransformCbuf = std::make_shared<TransformCbuf>(gfx, *this);
}

DirectX::XMMATRIX MeshRenderer::GetTransformXM() const
{
	return DirectX::XMLoadFloat4x4(&transform);
}

// Called via Node
void MeshRenderer::SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, dx::FXMMATRIX _accumulatedTranform) const
{
	dx::XMStoreFloat4x4(&transform, _accumulatedTranform);

	pMaterial->SubmitDrawCalls(frame, *this);
}

void MeshRenderer::Bind(Graphics& gfx) const
{
	pTopology->BindIA(gfx, 0u);
	pIndexBuffer->BindIA(gfx, 0u);
	pVertexBuffer->BindIA(gfx, 0u);
	pTransformCbuf->BindVS(gfx, 0u);
}

UINT MeshRenderer::GetIndexCount() const
{
	return pIndexBuffer->GetIndexCount();
}

UINT MeshRenderer::GetVertexCount() const
{
	return pVertexBuffer->GetVertexCount();
}

void MeshRenderer::IssueDrawCall(Graphics& gfx) const
{
	gfx.DrawIndexed(GetIndexCount());
}
