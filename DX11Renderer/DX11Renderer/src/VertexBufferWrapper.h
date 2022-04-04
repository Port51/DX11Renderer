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
		VertexBufferWrapper(GraphicsDevice& gfx, const BaseBufferData& vertexBuffer);
		VertexBufferWrapper(GraphicsDevice& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
	public:
		const ComPtr<ID3D11Buffer> GetVertexBuffer() const;
		void BindIA(GraphicsDevice& gfx, UINT slot);
		const UINT GetVertexCount() const;
	protected:
		void SetupVertexBuffer(GraphicsDevice& gfx, const BaseBufferData& data);
		void SetupInstanceBuffer(GraphicsDevice& gfx, const BaseBufferData& data);
	public:
		static std::shared_ptr<VertexBufferWrapper> Resolve(GraphicsDevice& gfx, std::string id, const BaseBufferData& vertexBuffer);
		static std::shared_ptr<VertexBufferWrapper> Resolve(GraphicsDevice& gfx, std::string id, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
	private:
		static std::string GenerateUID(const std::string& tag);
	protected:
		UINT m_vertexCount;
		std::vector<UINT> m_strides;
		std::vector<UINT> m_offsets;
		std::vector<ComPtr<ID3D11Buffer>> m_pBufferArray;
	};
}