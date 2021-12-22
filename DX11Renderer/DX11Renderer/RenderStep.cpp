#include "RenderStep.h"
#include "MeshRenderer.h"
#include "Renderer.h"

void RenderStep::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
{
	drawContext.renderer.AcceptDrawCall(DrawCall(this, &meshRenderer, drawContext), targetPass);
}

void RenderStep::InitializeParentReferences(const MeshRenderer& parent)
{
	for (const auto& b : bindings)
	{
		b.GetBindable()->InitializeParentReference(parent);
	}
}