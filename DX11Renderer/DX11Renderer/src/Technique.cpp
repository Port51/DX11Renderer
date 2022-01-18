#include "pch.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "DrawContext.h"

namespace gfx
{
	Technique::Technique(std::string name)
		:
		name(name)
	{}
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
	void Technique::AddStep(std::unique_ptr<RenderStep> step)
	{
		pSteps.push_back(std::move(step));
	}
	bool Technique::IsActive() const
	{
		return active;
	}
	void Technique::SetActiveState(bool active_in)
	{
		active = active_in;
	}
	const std::string & Technique::GetName() const
	{
		return name;
	}
}