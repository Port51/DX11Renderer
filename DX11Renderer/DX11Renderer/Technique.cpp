#include "Technique.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "DrawContext.h"

void Technique::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
{
	if (active)
	{
		for (const auto& step : pSteps)
		{
			step->SubmitDrawCalls(meshRenderer, drawContext);
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