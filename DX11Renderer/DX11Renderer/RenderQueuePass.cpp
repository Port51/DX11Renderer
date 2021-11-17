#include "RenderQueuePass.h"

namespace Rendergraph
{
	void RenderQueuePass::Accept(Job job)
	{
		jobs.push_back(job);
	}

	void RenderQueuePass::Execute(Graphics& gfx) const
	{
		BindAll(gfx);

		for (const auto& j : jobs)
		{
			j.Execute(gfx);
		}
	}

	void RenderQueuePass::Reset()
	{
		jobs.clear();
	}
}