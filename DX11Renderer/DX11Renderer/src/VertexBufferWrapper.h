#pragma once
#include "CodexElement.h"
#include "CommonHeader.h"
#include <vector>

struct ID3D11Buffer;

namespace gfx
{
	class GraphicsDevice;
	class BaseBufferData;

	class VertexBufferWrapper : public CodexElement
	{
	public:
		VertexBufferWrapper(const GraphicsDevice& gfx, const BaseBufferData& vertexBuffer);
		VertexBufferWrapper(const GraphicsDevice& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
		virtual void Release() override;

	public:
		const ComPtr<ID3D11Buffer> GetVertexBuffer() const;
		void BindIA(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		void UnbindIA(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot);
		const UINT GetVertexCount() const;
		void ApplyInstanceData(const GraphicsDevice& gfx, const BaseBufferData& instanceBuffer);

	protected:
		void SetupVertexBuffer(const GraphicsDevice& gfx, const BaseBufferData& data);
		void SetupInstanceBuffer(const GraphicsDevice& gfx, const BaseBufferData& data);

	public:
		static std::shared_ptr<VertexBufferWrapper> Resolve(const GraphicsDevice& gfx, const std::string id, const BaseBufferData& vertexBuffer);
		static std::shared_ptr<VertexBufferWrapper> Resolve(const GraphicsDevice& gfx, const std::string id, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);

	private:
		static std::string GenerateUID(const std::string& tag);

	protected:
		UINT m_vertexCount;
		std::vector<UINT> m_strides;
		std::vector<UINT> m_offsets;
		std::vector<ComPtr<ID3D11Buffer>> m_pBufferArray;

	};
}