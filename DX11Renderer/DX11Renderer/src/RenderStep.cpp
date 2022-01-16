#include "pch.h"
#include "RenderStep.h"
#include "MeshRenderer.h"
#include "Renderer.h"

namespace gfx
{
	void RenderStep::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		drawContext.renderer.AcceptDrawCall(DrawCall(this, &meshRenderer, drawContext), targetPass);
	}
}