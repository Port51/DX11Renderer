#pragma once

namespace gfx
{
	class MeshRenderer;
	class GraphicsDevice;
	class MaterialPass;
	struct DrawContext;

	class DrawCall
	{
	public:
		DrawCall(const MaterialPass* pMaterialPass, const MeshRenderer* pRenderer, const DrawContext& drawContext);
		virtual ~DrawCall() = default;
	public:
		void Execute(const GraphicsDevice& gfx) const;
	private:
		const MeshRenderer* m_pRenderer;
		const MaterialPass* m_pMaterialPass;
		const DrawContext* m_pDrawContext;
	};
}