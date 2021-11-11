#include "Step.h"
#include "MeshRenderer.h"
#include "FrameCommander.h"

void Step::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& meshRenderer) const
{
	frame.Accept(Job{ this,&meshRenderer }, targetPass);
}

void Step::InitializeParentReferences(const MeshRenderer& parent) noexcept
{
	for (auto& b : pBindables)
	{
		b->InitializeParentReference(parent);
	}
}