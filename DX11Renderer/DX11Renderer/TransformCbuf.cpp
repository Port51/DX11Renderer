#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
	:
	parent(parent)
{
	if (!pVcbuf)
	{
		pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
	}
}

void TransformCbuf::Bind(Graphics& gfx)
{
	const auto modelMatrix = parent.GetTransformXM();
	const auto modelViewMatrix = modelMatrix * gfx.GetViewMatrix();
	const auto modelViewProjectMatrix = modelViewMatrix * gfx.GetProjectionMatrix();
	const Transforms tf =
	{
		modelMatrix,
		modelViewMatrix,
		modelViewProjectMatrix
	};
	pVcbuf->Update(gfx, tf);
	pVcbuf->Bind(gfx);
}

// Because static
std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVcbuf;