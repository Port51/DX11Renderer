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

void MaterialPass::AddBinding(std::shared_ptr<Bindable> pBindable, UINT slot)
{
	bindings.emplace_back(Binding(pBindable, slot));
}

void MaterialPass::AddBinding(Binding binding)
{
	bindings.emplace_back(binding);
}

void MaterialPass::Bind(Graphics& gfx)
{
	pVertexShader->Bind(gfx);
	pPixelShader->Bind(gfx);
	for (const auto& binding : bindings)
	{
		binding.Bind(gfx);
	}
}

void MaterialPass::SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const
{
	pTechnique->SubmitDrawCalls(frame, renderer);
}
