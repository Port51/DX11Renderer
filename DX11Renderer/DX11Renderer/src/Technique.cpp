#include "pch.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "DrawContext.h"

namespace gfx
{
	Technique::Technique(std::string name)
		:
		m_name(name)
	{}
	void Technique::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		if (m_active)
		{
			for (const auto& step : m_pSteps)
			{
				step->SubmitDrawCalls(meshRenderer, drawContext);
			}
		}
	}
	void Technique::AddStep(std::unique_ptr<RenderStep> step)
	{
		m_pSteps.push_back(std::move(step));
	}
	bool Technique::IsActive() const
	{
		return m_active;
	}
	void Technique::SetActiveState(bool active_in)
	{
		m_active = active_in;
	}
	const std::string & Technique::GetName() const
	{
		return m_name;
	}
}