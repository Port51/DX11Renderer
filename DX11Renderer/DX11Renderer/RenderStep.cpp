#include "RenderStep.h"
#include "MeshRenderer.h"
#include "Renderer.h"

void RenderStep::SubmitDrawCalls(std::unique_ptr<Renderer>& frame, const MeshRenderer& meshRenderer) const
{
	frame->AcceptDrawCall(DrawCall(this, &meshRenderer), targetPass);
}

void RenderStep::InitializeParentReferences(const MeshRenderer& parent)
{
	for (const auto& b : bindings)
	{
		b.GetBindable()->InitializeParentReference(parent);
	}
}