#include "Job.h"
#include "Step.h"
#include "MeshRenderer.h"


Job::Job(const Step* pStep, const MeshRenderer* pRenderer)
	:
	pRenderer{ pRenderer },
	pStep{ pStep }
{}

void Job::Execute(Graphics& gfx) const
{
	pRenderer->Bind(gfx);
	pStep->Bind(gfx);
	gfx.DrawIndexed(pRenderer->GetIndexCount());
}