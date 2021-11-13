#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Binding.h"

MaterialPass::MaterialPass()
{
	
}

void MaterialPass::AddTechnique(std::unique_ptr<Technique> _pTechnique)
{
	pTechnique = std::move(_pTechnique);
}

void MaterialPass::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const
{
	pTechnique->SubmitDrawCalls(frame, renderer);
}
