#include "pch.h"
#include "DrawCommand.h"
#include "MaterialPass.h"
#include "Drawable.h"
#include "RenderPass.h"
#include "DrawContext.h"
#include "BindingList.h"

namespace gfx
{
	DrawCommand::DrawCommand(const MaterialPass* const pMaterialPass, const Drawable* const pDrawable, const DrawContext& _drawContext, const BindingList* const pPropertyBindings)
		: m_pDrawable(pDrawable),
		m_pMaterialPass(pMaterialPass),
		m_pDrawContext(&_drawContext),
		m_pPropertyBindings(pPropertyBindings)
	{}

	void DrawCommand::Execute(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		if (m_pPropertyBindings != nullptr)
		{
			m_pPropertyBindings->Bind(gfx, renderState);
		}

		m_pDrawable->Bind(gfx, renderState, *m_pDrawContext);
		m_pMaterialPass->Bind(gfx, renderState);
		m_pDrawable->IssueDrawCall(gfx); // calls DrawIndexed() or DrawIndexedInstanced()
	}
}