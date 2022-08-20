#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "ConstantBuffer.h"
#include "DXMathInclude.h"

namespace gfx
{
	class GraphicsDevice;
	class MeshRenderer;
	class ConstantBuffer;

	struct ObjectTransformsCB;

	class TransformCbuf : public Bindable
	{
	public:
		TransformCbuf(const GraphicsDevice& gfx);
		virtual void Release() override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UpdateTransforms(const GraphicsDevice& gfx, const ObjectTransformsCB& transforms);
	private:
		// Static so can be re-used each drawcall
		static std::unique_ptr<ConstantBuffer> m_pVcbuf;
	};
}