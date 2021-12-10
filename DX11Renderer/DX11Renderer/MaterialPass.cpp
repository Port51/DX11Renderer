#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "Binding.h"
//#include "VertexShader.h"
//#include "PixelShader.h"

MaterialPass::MaterialPass()
{
	
}

void MaterialPass::AddTechnique(std::unique_ptr<Technique> _pTechnique)
{
	pTechnique = std::move(_pTechnique);
}

void MaterialPass::SubmitDrawCalls(std::unique_ptr<Renderer>& frame, const MeshRenderer& renderer) const
{
	pTechnique->SubmitDrawCalls(frame, renderer);
}
