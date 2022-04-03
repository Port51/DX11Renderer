#include "pch.h"
#include "DrawCall.h"
#include "RenderStep.h"
#include "MeshRenderer.h"
#include "RenderPass.h"
#include "DrawContext.h"

namespace gfx
{
	DrawCall::DrawCall(const RenderStep* pStep, const MeshRenderer* pRenderer, const DrawContext& _drawContext)
		: m_pRenderer(pRenderer),
		m_pStep(pStep),
		m_pDrawContext(&_drawContext)
	{}

	void DrawCall::Execute(GraphicsDevice& gfx) const
	{
		m_pRenderer->Bind(gfx, *m_pDrawContext);
		m_pStep->Bind(gfx);
		m_pRenderer->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
	}
}