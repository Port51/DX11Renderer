#include "RenderStep.h"
#include "MeshRenderer.h"
#include "FrameCommander.h"

void RenderStep::SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, const MeshRenderer& meshRenderer) const
{
	frame->Accept(RenderJob(this, &meshRenderer), targetPass);
}

void RenderStep::InitializeParentReferences(const MeshRenderer& parent)
{
	for (const auto& b : bindings)
	{
		b.GetBindable()->InitializeParentReference(parent);
	}
}