#include "Technique.h"
#include "MeshRenderer.h"
#include "FrameCommander.h"

namespace Rgph
{
	void Technique::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const
	{
		if (active)
		{
			for (const auto& step : pSteps)
			{
				step->SubmitDrawCalls(frame, renderer);
			}
		}
	}

	void Technique::InitializeParentReferences(const MeshRenderer & parent)
	{
		for (auto& s : pSteps)
		{
			s->InitializeParentReferences(parent);
		}
	}
}