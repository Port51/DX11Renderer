#include "TransformCbuf.h"
#include "MeshRenderer.h"
#include "Graphics.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const MeshRenderer& parent)
	: pParent(&parent)
{
	if (!pVcbuf)
	{
		pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, "NotInCodex");
	}
}

///
/// Update cbuffer based on transform, then bind
///
void TransformCbuf::Bind(Graphics& gfx, UINT slot)
{
	UpdateBindImpl(gfx, GetTransforms(gfx));
}

void TransformCbuf::InitializeParentReference(const MeshRenderer& parent)
{
	pParent = &parent;
}

void TransformCbuf::UpdateBindImpl(Graphics& gfx, const Transforms& transforms)
{
	assert(pParent != nullptr);
	pVcbuf->Update(gfx, transforms);
	pVcbuf->Bind(gfx, 0u);
}

TransformCbuf::Transforms TransformCbuf::GetTransforms(Graphics& gfx)
{
	assert(pParent != nullptr);
	const auto modelMatrix = pParent->GetTransformXM();
	const auto modelViewMatrix = modelMatrix * gfx.GetViewMatrix();
	const auto modelViewProjectMatrix = modelViewMatrix * gfx.GetProjectionMatrix();
	return Transforms
	{
		modelMatrix,
		modelViewMatrix,
		modelViewProjectMatrix
	};
}

// Because static
std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVcbuf;