#include "pch.h"
#include "VertexBufferWrapper.h"
#include "BaseBufferData.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	VertexBufferWrapper::VertexBufferWrapper(const GraphicsDevice& gfx, const BaseBufferData& vertexBuffer)
	{
		m_strides.resize(1);
		m_offsets.resize(1);
		m_pBufferArray.resize(1);
		SetupVertexBuffer(gfx, vertexBuffer);
	}

	VertexBufferWrapper::VertexBufferWrapper(const GraphicsDevice& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer)
	{
		m_strides.resize(2);
		m_offsets.resize(2);
		m_pBufferArray.resize(2);
		SetupVertexBuffer(gfx, vertexBuffer);
		SetupInstanceBuffer(gfx, instanceBuffer);
	}

	void VertexBufferWrapper::Release()
	{
		for (int i = 0, ct = m_pBufferArray.size(); i < ct; ++i)
		{
			m_pBufferArray[i].Reset();
		}
	}

	const ComPtr<ID3D11Buffer> VertexBufferWrapper::GetVertexBuffer() const
	{
		return m_pBufferArray[0];
	}

	void VertexBufferWrapper::BindIA(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::IA_VertexBuffer, slot))
		{
			gfx.GetContext()->IASetVertexBuffers(slot, m_pBufferArray.size(), m_pBufferArray[0].GetAddressOf(), &m_strides[0], &m_offsets[0]);
		}
	}

	void VertexBufferWrapper::UnbindIA(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		// todo: bind null array
		renderState.ClearBinding(RenderBindingType::IA_VertexBuffer, slot);
		//gfx.GetContext()->IASetVertexBuffers(slot, m_pBufferArray.size(), m_pBufferArray[0].GetAddressOf(), &m_strides[0], &m_offsets[0]);
	}

	void VertexBufferWrapper::SetupVertexBuffer(const GraphicsDevice& gfx, const BaseBufferData& data)
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(data.GetSizeInBytes());
		bd.StructureByteStride = data.GetStride();

		m_strides[0] = data.GetStride();
		m_offsets[0] = 0;

		assert(data.GetStride() % 16 == 0 && "Vertex buffer stride must be a multiple of 16");

		D3D11_SUBRESOURCE_DATA sd = data.GetSubresourceData();
		gfx.GetAdapter()->CreateBuffer(&bd, &sd, &m_pBufferArray[0]);

		m_vertexCount = data.GetElementCount();
	}

	void VertexBufferWrapper::SetupInstanceBuffer(const GraphicsDevice& gfx, const BaseBufferData& data)
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = UINT(data.GetSizeInBytes());
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.StructureByteStride = data.GetStride();

		m_strides[1] = data.GetStride();
		m_offsets[1] = 0;

		assert(data.GetStride() % 16 == 0 && "Instance buffer stride must be a multiple of 16");

		D3D11_SUBRESOURCE_DATA sd = data.GetSubresourceData();
		gfx.GetAdapter()->CreateBuffer(&bd, &sd, &m_pBufferArray[1]);
	}

	std::shared_ptr<VertexBufferWrapper> VertexBufferWrapper::Resolve(const GraphicsDevice& gfx, std::string id, const BaseBufferData& vertexBuffer)
	{
		return std::move(Codex::Resolve<VertexBufferWrapper>(gfx, GenerateUID(id), vertexBuffer));
	}

	std::shared_ptr<VertexBufferWrapper> VertexBufferWrapper::Resolve(const GraphicsDevice& gfx, std::string id, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer)
	{
		return std::move(Codex::Resolve<VertexBufferWrapper>(gfx, GenerateUID(id), vertexBuffer, instanceBuffer));
	}

	std::string VertexBufferWrapper::GenerateUID(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(VertexBufferWrapper).name() + "#"s + tag;
	}

	const UINT VertexBufferWrapper::GetVertexCount() const
	{
		return m_vertexCount;
	}
}