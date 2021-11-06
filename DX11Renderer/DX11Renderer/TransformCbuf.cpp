#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
	:
	parent(parent)
{
	if (!pVcbuf)
	{
		pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, "NotInCodex", slot);
	}
}

///
/// Update cbuffer based on transform, then bind
///
void TransformCbuf::Bind(Graphics& gfx)
{
	UpdateBindImpl(gfx, GetTransforms(gfx));
}

void TransformCbuf::UpdateBindImpl(Graphics& gfx, const Transforms& transforms)
{
	pVcbuf->Update(gfx, transforms);
	pVcbuf->Bind(gfx);
}

TransformCbuf::Transforms TransformCbuf::GetTransforms(Graphics& gfx)
{
	const auto modelMatrix = parent.GetTransformXM();
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