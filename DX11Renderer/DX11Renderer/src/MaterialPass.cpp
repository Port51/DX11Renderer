#include "pch.h"
#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "Binding.h"
#include "DrawContext.h"
//#include "VertexShader.h"
//#include "PixelShader.h"

namespace gfx
{
	MaterialPass::MaterialPass()
	{

	}

	void MaterialPass::SetRenderPass(std::string _renderPass)
	{
		m_renderPass = _renderPass;
	}

	const std::string MaterialPass::GetRenderPass() const
	{
		return m_renderPass;
	}

	void MaterialPass::AddTechnique(std::unique_ptr<Technique> _pTechnique)
	{
		m_pTechnique = std::move(_pTechnique);
	}

	void MaterialPass::SubmitDrawCommands(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const
	{
		m_pTechnique->SubmitDrawCalls(meshRenderer, drawContext);
	}
}