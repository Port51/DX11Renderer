#pragma once
#include "GameObject.h"

namespace gfx
{
	class GraphicsDevice;
	class RenderState;

	struct DrawContext;

	class Drawable //: public GameObject
	{
	public:
		virtual void Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const = 0;
		virtual void IssueDrawCall(const GraphicsDevice& gfx) const = 0;
	};
}