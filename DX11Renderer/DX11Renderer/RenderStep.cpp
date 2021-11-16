#include "RenderStep.h"
#include "MeshRenderer.h"
#include "FrameCommander.h"
#include "Graphics.h"

namespace Rgph
{
	RenderStep::RenderStep(std::string _targetPass)
		: targetPass(std::move(_targetPass))
	{}
	RenderStep::RenderStep(const RenderStep & src)
		: targetPass(src.targetPass)
	{
		bindings.reserve(src.bindings.size());
		for (auto& pb : src.bindings)
		{
			if (auto* pCloning = dynamic_cast<const Bind::CloningBindable*>(pb.get()))
			{
				bindings.push_back(pCloning->Clone());
			}
			else
			{
				bindings.push_back(pb);
			}
		}
	}
	void RenderStep::AddBinding(std::shared_ptr<Bind::Bindable> pBindable, UINT slot)
	{
		bindings.push_back(Bind::Binding(std::move(pBindable), slot));
	}
	void RenderStep::AddBinding(Bind::Binding pBinding)
	{
		bindings.push_back(std::move(pBinding));
	}
	void RenderStep::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& meshRenderer) const
	{
		frame.Accept(RenderJob{ this, &meshRenderer }, targetPass);
	}

	void RenderStep::Bind(Graphics & gfx) const
	{
		for (const auto& b : bindings)
		{
			b.Bind(gfx);
		}
	}

	void RenderStep::InitializeParentReferences(const MeshRenderer& parent)
	{
		for (const auto& b : bindings)
		{
			b.GetBindable()->InitializeParentReference(parent);
		}
	}
}