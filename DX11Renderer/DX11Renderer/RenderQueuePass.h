#pragma once
#include "BindingPass.h"
#include "RenderJob.h"
#include <vector>

namespace Rendergraph
{
	class RenderQueuePass : public BindingPass
	{
	public:
		using BindingPass::BindingPass;
		void Accept(RenderJob job);
		void Execute(Graphics& gfx) const override;
		void Reset() override;
	private:
		std::vector<RenderJob> jobs;
	};
}