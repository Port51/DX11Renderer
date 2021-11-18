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
	RenderPass()
	{

	}
	void EnqueueJob(RenderJob job);
	virtual void Execute(Graphics& gfx) const;
	void Reset();
private:
	std::vector<RenderJob> jobs; // will be replaced by render graph
};