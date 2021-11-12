#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "VertexShader.h"
#include "PixelShader.h"

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
	pVertexShader->Bind(gfx);
	pPixelShader->Bind(gfx);
	for (const auto& bindable : pBindables)
	{
		bindable->Bind(gfx);
	}
}

void MaterialPass::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const
{
	pTechnique->SubmitDrawCalls(frame, renderer);
}
