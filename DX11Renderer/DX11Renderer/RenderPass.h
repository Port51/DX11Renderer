#pragma once
#include "Graphics.h"
#include "RenderJob.h"
#include <vector>

///
/// Wrapper containing jobs
///
class RenderPass
{
public:
	void EnqueueJob(RenderJob job) noexcept
	{
		jobs.push_back(job);
	}
	void Execute(Graphics& gfx) const
	{
		for (const auto& j : jobs)
		{
			j.Execute(gfx);
		}
	}
	void Reset() noexcept
	{
		jobs.clear();
	}
private:
	std::vector<RenderJob> jobs; // will be replaced by render graph
};