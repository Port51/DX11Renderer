#pragma once
#include "CommonHeader.h"
#include "Buffer.h"

enum D3D11_USAGE;

namespace gfx
{
	class GraphicsDevice;

	class ConstantBuffer : public Buffer
	{
	public:
		ConstantBuffer(const GraphicsDevice& gfx, const D3D11_USAGE usage, const UINT bufferSizeBytes);
		ConstantBuffer(const GraphicsDevice& gfx, const D3D11_USAGE usage, const void* initialData, const UINT bufferSizeBytes);

	public:
		void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void Update(const GraphicsDevice& gfx, const void* data);

	private:
		static constexpr UINT GetCBufferSize(const UINT buffer_size);
	};
}