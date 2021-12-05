#include "RenderJob.h"
#include "RenderStep.h"
#include "MeshRenderer.h"
#include "RenderPass.h"


RenderJob::RenderJob(const RenderStep* pStep, const MeshRenderer* pRenderer)
	:
	pRenderer{ pRenderer },
	pStep{ pStep }
{}

void RenderJob::Execute(Graphics& gfx, const RenderPass& renderPass) const
{
	pRenderer->Bind(gfx, renderPass);
	pStep->Bind(gfx, renderPass);
	pRenderer->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
}