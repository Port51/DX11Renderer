#include "MaterialPass.h"
#include "Technique.h"
#include "MeshRenderer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Binding.h"

namespace Bind
{
	MaterialPass::MaterialPass()
	{

	}

	void MaterialPass::AddTechnique(std::unique_ptr<Rendergraph::Technique> _pTechnique)
	{
		pTechnique = std::move(_pTechnique);
	}

	void MaterialPass::SubmitDrawCalls(Rendergraph::FrameCommander& frame, const MeshRenderer& renderer) const
	{
		pTechnique->SubmitDrawCalls(frame, renderer);
	}

}