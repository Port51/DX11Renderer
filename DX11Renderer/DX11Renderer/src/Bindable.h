#pragma once
#include "CodexElement.h"
#include "CommonHeader.h"

struct ID3D11DeviceContext;
struct ID3D11Device;

namespace gfx
{
	class GraphicsDevice;
	class MeshRenderer;
	class RenderPass;
	class RenderState;

	class Bindable : public CodexElement
	{
	public:
		virtual void BindIA(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void BindHS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void BindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void BindRS(const GraphicsDevice& gfx, RenderState& renderState);
		virtual void BindOM(const GraphicsDevice& gfx, RenderState& renderState);
		virtual void UnbindIA(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void UnbindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void UnbindHS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void UnbindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		virtual void UnbindRS(const GraphicsDevice& gfx, RenderState& renderState);
		virtual void UnbindOM(const GraphicsDevice& gfx, RenderState& renderState);
		virtual ~Bindable();
		virtual void Release() override = 0;
	};
}