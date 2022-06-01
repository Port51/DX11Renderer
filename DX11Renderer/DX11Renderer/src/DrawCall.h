#pragma once

namespace gfx
{
	class MeshRenderer;
	class GraphicsDevice;
	class MaterialPass;
	class BindingList;

	struct DrawContext;

	class DrawCall
	{
	public:
		DrawCall(const MaterialPass* const pMaterialPass, const MeshRenderer* const pRenderer, const DrawContext& drawContext, const BindingList* const pPropertyBindings);
		virtual ~DrawCall() = default;
	public:
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const;
	private:
		const MeshRenderer* const m_pRenderer;
		const MaterialPass* const m_pMaterialPass;
		const DrawContext* const m_pDrawContext;
		const BindingList* const m_pPropertyBindings;
	};
}