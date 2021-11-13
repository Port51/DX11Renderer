#include "RenderStep.h"
#include "MeshRenderer.h"
#include "FrameCommander.h"

void RenderStep::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& meshRenderer) const
{
	frame.Accept(RenderJob{ this,&meshRenderer }, targetPass);
}

void RenderStep::InitializeParentReferences(const MeshRenderer& parent) noexcept
{
	for (const auto& b : bindings)
	{
		b.GetBindable()->InitializeParentReference(parent);
	}
}