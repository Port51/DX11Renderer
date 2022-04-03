#pragma once

namespace gfx
{
	class RenderStep;
	class RenderPass;
	class MeshRenderer;
	class GraphicsDevice;
	struct DrawContext;

	class DrawCall
	{
	public:
		DrawCall(const RenderStep* pStep, const MeshRenderer* pRenderer, const DrawContext& drawContext);
		virtual ~DrawCall() = default;
	public:
		void Execute(GraphicsDevice& gfx) const;
	private:
		const MeshRenderer* m_pRenderer;
		const RenderStep* m_pStep;
		const DrawContext* m_pDrawContext;
	};
}