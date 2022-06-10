#pragma once

namespace gfx
{
	class GraphicsDevice;
	class RenderState;

	struct DrawContext;

	class Drawable
	{
	public:
		virtual void Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const = 0;
		virtual void IssueDrawCall(const GraphicsDevice& gfx) const = 0;
	};
}