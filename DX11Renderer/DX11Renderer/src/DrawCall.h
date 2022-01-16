#pragma once

namespace gfx
{
	class RenderStep;
	class RenderPass;
	class MeshRenderer;
	class Graphics;
	struct DrawContext;

	class DrawCall
	{
	public:
		DrawCall(const RenderStep* pStep, const MeshRenderer* pRenderer, const DrawContext& drawContext);
		void Execute(Graphics& gfx) const;
	private:
		const MeshRenderer* pRenderer;
		const RenderStep* pStep;
		const DrawContext* drawContext;
	};
}