#include "RenderPass.h"

void RenderPass::EnqueueJob(RenderJob job)
{
	jobs.push_back(job);
}

void RenderPass::Execute(Graphics & gfx) const
{
	for (const auto& j : jobs)
	{
		j.Execute(gfx);
	}
}

void RenderPass::Reset()
{
	jobs.clear();
}
