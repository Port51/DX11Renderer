#include "Technique.h"
#include "MeshRenderer.h"
#include "FrameCommander.h"

void Technique::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const noexcept
{
	if (active)
	{
		for (const auto& step : steps)
		{
			step.SubmitDrawCalls(frame, renderer);
		}
	}
}

void Technique::InitializeParentReferences(const MeshRenderer & parent) noexcept
{
	for (auto& s : steps)
	{
		s.InitializeParentReferences(parent);
	}
}