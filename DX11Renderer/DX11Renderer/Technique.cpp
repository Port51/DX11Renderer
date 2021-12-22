#include "Technique.h"
#include "MeshRenderer.h"
#include "Renderer.h"

void Technique::SubmitDrawCalls(Renderer& renderer, const MeshRenderer& meshRenderer) const
{
	if (active)
	{
		for (const auto& step : pSteps)
		{
			step->SubmitDrawCalls(renderer, meshRenderer);
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