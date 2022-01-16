#include "pch.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "DrawContext.h"

namespace gfx
{
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
}