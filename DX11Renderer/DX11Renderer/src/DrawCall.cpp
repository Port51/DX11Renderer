#include "pch.h"
#include "DrawCall.h"
#include "MaterialPass.h"
#include "MeshRenderer.h"
#include "RenderPass.h"
#include "DrawContext.h"

namespace gfx
{
	DrawCall::DrawCall(const MaterialPass* pMaterialPass, const MeshRenderer* pRenderer, const DrawContext& _drawContext)
		: m_pRenderer(pRenderer),
		m_pMaterialPass(pMaterialPass),
		m_pDrawContext(&_drawContext)
	{}

	void DrawCall::Execute(const GraphicsDevice& gfx) const
	{
		m_pRenderer->Bind(gfx, *m_pDrawContext);
		m_pMaterialPass->Bind(gfx);
		m_pRenderer->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
	}
}