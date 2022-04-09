#include "pch.h"
#include "InputLayout.h"
#include "SharedCodex.h"
#include "VertexLayout.h"

namespace gfx
{
	InputLayout::InputLayout(const GraphicsDevice& gfx, VertexLayout _layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode)
		: m_layout(std::move(_layout)),
		m_vertexShaderName(vertexShaderName)
	{
		const auto d3dLayout = m_layout.GetD3DLayout();
		THROW_IF_FAILED(gfx.GetAdapter()->CreateInputLayout(
			d3dLayout.data(), (UINT)d3dLayout.size(),
			pVertexShaderBytecode->GetBufferPointer(),
			pVertexShaderBytecode->GetBufferSize(),
			&m_pInputLayout
		));
	}

	void InputLayout::Release()
	{
		m_pInputLayout.Reset();
	}

	void InputLayout::BindIA(const GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->IASetInputLayout(m_pInputLayout.Get());
	}

	std::shared_ptr<InputLayout> InputLayout::Resolve(const GraphicsDevice& gfx, const VertexLayout& layout, std::string vertexShaderName, ID3DBlob * pVertexShaderBytecode)
	{
		return std::move(Codex::Resolve<InputLayout>(gfx, GenerateUID(layout, vertexShaderName, pVertexShaderBytecode), layout, vertexShaderName, pVertexShaderBytecode));
	}

	std::string InputLayout::GenerateUID(const VertexLayout& layout, std::string vertexShaderName, ID3DBlob * pVertexShaderBytecode)
	{
		using namespace std::string_literals;
		return typeid(InputLayout).name() + "#"s + layout.GetCode() + "|" + vertexShaderName;
	}
}