#include "pch.h"
#include "DrawCall.h"
#include "MaterialPass.h"
#include "MeshRenderer.h"
#include "RenderPass.h"
#include "DrawContext.h"
#include "BindingList.h"

namespace gfx
{
	DrawCall::DrawCall(const MaterialPass* const pMaterialPass, const MeshRenderer* const pRenderer, const DrawContext& _drawContext, const BindingList* const pPropertyBindings)
		: m_pRenderer(pRenderer),
		m_pMaterialPass(pMaterialPass),
		m_pDrawContext(&_drawContext),
		m_pPropertyBindings(pPropertyBindings)
	{}

	void DrawCall::Execute(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		if (m_pPropertyBindings != nullptr)
		{
			m_pPropertyBindings->Bind(gfx, renderState);
		}

		m_pRenderer->Bind(gfx, renderState, *m_pDrawContext);
		m_pMaterialPass->Bind(gfx, renderState);
		m_pRenderer->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
	}
}