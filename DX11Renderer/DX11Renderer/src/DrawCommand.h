#pragma once

namespace gfx
{
	class Drawable;
	class GraphicsDevice;
	class MaterialPass;
	class BindingList;

	struct DrawContext;

	class DrawCommand
	{
	public:
		DrawCommand(const MaterialPass* const pMaterialPass, const Drawable* const pDrawable, const DrawContext& drawContext, const BindingList* const pPropertyBindings);
		virtual ~DrawCommand();
	public:
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const;
	private:
		const Drawable* const m_pDrawable;
		const MaterialPass* const m_pMaterialPass;
		const DrawContext* const m_pDrawContext;
		const BindingList* const m_pPropertyBindings;
	};
}