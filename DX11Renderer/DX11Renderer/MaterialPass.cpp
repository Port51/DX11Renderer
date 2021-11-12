#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"

MaterialPass::MaterialPass()
{
	
}

void MaterialPass::AddTechnique(std::unique_ptr<Technique> _pTechnique)
{
	pTechnique = std::move(_pTechnique);
}

void MaterialPass::AddBindable(std::shared_ptr<Bindable> pBindable)
{
	pBindables.emplace_back(pBindable);
}

void MaterialPass::Bind(Graphics& gfx)
{
	for (const auto& bindable : pBindables)
	{
		bindable->Bind(gfx);
	}
}

void MaterialPass::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const
{
	pTechnique->SubmitDrawCalls(frame, renderer);
}
