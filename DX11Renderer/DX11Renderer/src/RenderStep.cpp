#include "pch.h"
#include "RenderStep.h"
#include "GraphicsDevice.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "DrawContext.h"
#include "DrawCall.h"
#include "Bindable.h"
#include "Binding.h"

namespace gfx
{
	RenderStep::RenderStep(std::string _targetPass)
		: m_targetPass{ _targetPass }
	{}
	Binding& RenderStep::AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		m_bindings.push_back(Binding(std::move(pBindable)));
		return m_bindings[m_bindings.size() - 1];
	}
	Binding& RenderStep::AddBinding(Binding pBinding)
	{
		m_bindings.push_back(std::move(pBinding));
		return m_bindings[m_bindings.size() - 1];
	}
	void RenderStep::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		drawContext.renderer.AcceptDrawCall(DrawCall(this, &meshRenderer, drawContext), m_targetPass);
	}
	void RenderStep::Bind(const GraphicsDevice& gfx) const
	{
		for (const auto& b : m_bindings)
		{
			b.Bind(gfx);
		}
	}
}