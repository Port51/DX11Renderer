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
		: targetPass{ _targetPass }
	{}
	Binding & RenderStep::AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		bindings.push_back(Binding(std::move(pBindable)));
		return bindings[bindings.size() - 1];
	}
	Binding & RenderStep::AddBinding(Binding pBinding)
	{
		bindings.push_back(std::move(pBinding));
		return bindings[bindings.size() - 1];
	}
	void RenderStep::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		drawContext.renderer.AcceptDrawCall(DrawCall(this, &meshRenderer, drawContext), targetPass);
	}
	void RenderStep::Bind(GraphicsDevice & gfx) const
	{
		for (const auto& b : bindings)
		{
			b.Bind(gfx);
		}
	}
}