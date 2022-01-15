#include "pch.h"
#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "Binding.h"
#include "DrawContext.h"
//#include "VertexShader.h"
//#include "PixelShader.h"

MaterialPass::MaterialPass()
{
	
}

void MaterialPass::SetRenderPass(std::string _renderPass)
{
	renderPass = _renderPass;
}

const std::string MaterialPass::GetRenderPass() const
{
	return renderPass;
}

void MaterialPass::AddTechnique(std::unique_ptr<Technique> _pTechnique)
{
	pTechnique = std::move(_pTechnique);
}

void MaterialPass::SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
{
	pTechnique->SubmitDrawCalls(meshRenderer, drawContext);
}
