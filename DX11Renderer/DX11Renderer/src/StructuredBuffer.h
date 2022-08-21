#pragma once
#include "Buffer.h"
#include "DX11Include.h"
#include "GraphicsDevice.h"

namespace gfx
{
	class StructuredBuffer : public Buffer
	{
	public:
		StructuredBuffer(const GraphicsDevice& gfx, const D3D11_USAGE usage, const UINT bindFlags, const UINT numElements, const UINT elementSizeBytes, const void* initialData, const bool useCounter = false);
		StructuredBuffer(const GraphicsDevice& gfx, const D3D11_USAGE usage, const UINT bindFlags, const UINT numElements, const UINT elementSizeBytes, const bool useCounter = false);

	public:
		void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void Update(const GraphicsDevice& gfx, const void* data);
		void Update(const GraphicsDevice& gfx, const void* data, const size_t numElements);

	protected:
		const UINT GetDataBytes() const;

	protected:
		bool m_useCounter;
		UINT m_numElements;

	};
}