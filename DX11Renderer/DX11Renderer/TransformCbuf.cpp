#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent)
	:
	parent(parent)
{
	if (!pVcbuf)
	{
		pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx);
	}
}

void TransformCbuf::Bind(Graphics& gfx) noexcept
{
	const auto model = parent.GetTransformXM();
	const Transforms tf =
	{
		model,
		model * gfx.GetViewMatrix() * gfx.GetProjectionMatrix()
	};
	pVcbuf->Update(gfx, tf);
	pVcbuf->Bind(gfx);
}

// Because static
std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVcbuf;