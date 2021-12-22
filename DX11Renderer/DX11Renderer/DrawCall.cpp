#include "DrawCall.h"
#include "RenderStep.h"
#include "MeshRenderer.h"
#include "RenderPass.h"


DrawCall::DrawCall(const RenderStep* pStep, const MeshRenderer* pRenderer)
	:
	pRenderer{ pRenderer },
	pStep{ pStep }
{}

void DrawCall::Execute(Graphics& gfx) const
{
	pRenderer->Bind(gfx);
	pStep->Bind(gfx);
	pRenderer->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
}