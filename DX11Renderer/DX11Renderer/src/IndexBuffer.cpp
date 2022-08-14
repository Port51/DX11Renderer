#include "pch.h"
#include "IndexBuffer.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	IndexBuffer::IndexBuffer(const GraphicsDevice& gfx, const std::vector<u16>& indices, const bool useQuads)
		: m_count((UINT)indices.size()),
		m_format(DXGI_FORMAT_R16_UINT)
	{
		D3D11_BUFFER_DESC ibd;
		ZERO_MEM(ibd);
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = UINT(m_count * sizeof(u16));
		ibd.StructureByteStride = sizeof(u16);

		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = (useQuads) ? ConvertTrisToQuads(indices).data() : indices.data();

		THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&ibd, &isd, &m_pIndexBuffer));
	}

	IndexBuffer::IndexBuffer(const GraphicsDevice& gfx, const std::vector<u32>& indices, const bool useQuads)
		: m_count((UINT)indices.size()),
		m_format(DXGI_FORMAT_R32_UINT)
	{
		D3D11_BUFFER_DESC ibd = {};
		ZERO_MEM(ibd);
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = UINT(m_count * sizeof(u32));
		ibd.StructureByteStride = sizeof(u32);

		D3D11_SUBRESOURCE_DATA isd = {};

		if (useQuads)
		{
			auto quadIndices = ConvertTrisToQuads(indices);
			isd.pSysMem = quadIndices.data();
			THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&ibd, &isd, &m_pIndexBuffer));
		}
		else
		{
			isd.pSysMem = indices.data();
			THROW_IF_FAILED(gfx.GetAdapter()->CreateBuffer(&ibd, &isd, &m_pIndexBuffer));
		}
	}

	void IndexBuffer::Release()
	{
		m_pIndexBuffer.Reset();
	}

	void IndexBuffer::BindIA(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::IA_IndexBuffer, slot))
		{
			gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), m_format, 0u);
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void IndexBuffer::UnbindIA(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::IA_IndexBuffer, slot);
		gfx.GetContext()->IASetIndexBuffer(nullptr, m_format, 0u);
		REGISTER_GPU_CALL();
	}

	const UINT IndexBuffer::GetIndexCount() const
	{
		return m_count;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Resolve(const GraphicsDevice& gfx, const std::string id, const std::vector<u32>& indices, const bool useQuads)
	{
		return std::move(Codex::Resolve<IndexBuffer>(gfx, GenerateUID(id, useQuads), indices, useQuads));
	}

	std::string IndexBuffer::GenerateUID(const std::string& tag, const bool useQuads)
	{
		using namespace std::string_literals;
		return typeid(IndexBuffer).name() + "#"s + tag + (useQuads ? "-quad" : "-tri");
	}

	const std::vector<u16> IndexBuffer::ConvertTrisToQuads(const std::vector<u16>& indices) const
	{
		if (indices.size() % 6u != 0u) THROW("Cannot convert indices to quads!");

		const size_t quadCt = indices.size() / 6u;
		std::vector<u16> quadIndices;
		quadIndices.reserve(quadCt * 4u);

		for (int i = 0; i < quadCt; ++i)
		{
			// Unwind triangles
			quadIndices.emplace_back(indices.at(i * 6u + 2u));
			quadIndices.emplace_back(indices.at(i * 6u + 5u));
			quadIndices.emplace_back(indices.at(i * 6u + 0u));
			quadIndices.emplace_back(indices.at(i * 6u + 1u));
		}

		return quadIndices;
	}

	const std::vector<u32> IndexBuffer::ConvertTrisToQuads(const std::vector<u32>& indices) const
	{

		if (indices.size() % 6u != 0u) THROW("Cannot convert indices to quads!");

		const size_t quadCt = indices.size() / 6u;
		std::vector<u32> quadIndices;
		quadIndices.reserve(quadCt * 4u);

		for (int i = 0; i < quadCt; ++i)
		{
			// Unwind triangles
			quadIndices.emplace_back(indices.at(i * 6u + 2u));
			quadIndices.emplace_back(indices.at(i * 6u + 5u));
			quadIndices.emplace_back(indices.at(i * 6u + 0u));
			quadIndices.emplace_back(indices.at(i * 6u + 1u));
		}

		return quadIndices;
	}
}