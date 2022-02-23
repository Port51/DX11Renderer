#pragma once
#include "CodexElement.h"
#include "CommonHeader.h"
#include <vector>

struct ID3D11Buffer;

namespace gfx
{
	class Graphics;
	class BaseBufferData;

	class VertexBufferWrapper : public CodexElement
	{
	public:
		VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer);
		VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
	public:
		ComPtr<ID3D11Buffer> GetVertexBuffer() const;
		void BindIA(Graphics& gfx, UINT slot);
		UINT GetVertexCount() const;
	protected:
		void SetupVertexBuffer(Graphics& gfx, const BaseBufferData& data);
		void SetupInstanceBuffer(Graphics& gfx, const BaseBufferData& data);
	public:
		static std::shared_ptr<VertexBufferWrapper> Resolve(Graphics& gfx, std::string id, const BaseBufferData& vertexBuffer);
		static std::shared_ptr<VertexBufferWrapper> Resolve(Graphics& gfx, std::string id, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
	private:
		static std::string GenerateUID(const std::string& tag);
	protected:
		UINT vertexCount;
		std::vector<UINT> strides;
		std::vector<UINT> offsets;
		std::vector<ComPtr<ID3D11Buffer>> pBufferArray;
	};
}