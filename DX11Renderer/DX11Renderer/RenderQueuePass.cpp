#include "RenderQueuePass.h"

namespace Rgph
{
	void RenderQueuePass::Accept(RenderJob job)
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