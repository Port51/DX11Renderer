#include "RenderJob.h"
#include "RenderStep.h"
#include "MeshRenderer.h"


RenderJob::RenderJob(const RenderStep* pStep, const MeshRenderer* pRenderer)
	:
	pRenderer{ pRenderer },
	pStep{ pStep }
{}

void RenderJob::Execute(Graphics& gfx) const
{
	pRenderer->Bind(gfx);
	pStep->Bind(gfx);
	//gfx.DrawIndexed(pRenderer->GetIndexCount());
	gfx.pContext->DrawIndexedInstanced(pRenderer->GetIndexCount(), 10u, 0u, 0, 0u);
}