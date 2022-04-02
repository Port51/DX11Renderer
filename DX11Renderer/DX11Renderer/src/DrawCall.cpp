#include "pch.h"
#include "DrawCall.h"
#include "RenderStep.h"
#include "MeshRenderer.h"
#include "RenderPass.h"
#include "DrawContext.h"

namespace gfx
{
	DrawCall::DrawCall(const RenderStep* pStep, const MeshRenderer* pRenderer, const DrawContext& _drawContext)
		: pRenderer(pRenderer),
		pStep(pStep),
		drawContext(&_drawContext)
	{}

	void DrawCall::Execute(GraphicsDevice& gfx) const
	{
		pRenderer->Bind(gfx, *drawContext);
		pStep->Bind(gfx);
		pRenderer->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
	}
}