#include "InputLayout.h"
#include "GraphicsThrowMacros.h"
#include "SharedCodex.h"
#include "VertexLayout.h"

InputLayout::InputLayout(Graphics& gfx, VertexLayout _layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode)
	: layout(std::move(_layout)),
	vertexShaderName(vertexShaderName)
{
	SETUP_LOGGING(gfx);

	const auto d3dLayout = layout.GetD3DLayout();
	GFX_THROW_INFO(gfx.GetDevice()->CreateInputLayout(
		d3dLayout.data(), (UINT)d3dLayout.size(),
		pVertexShaderBytecode->GetBufferPointer(),
		pVertexShaderBytecode->GetBufferSize(),
		&pInputLayout
	));
}

void InputLayout::BindIA(Graphics& gfx, UINT slot)
{
	gfx.GetContext()->IASetInputLayout(pInputLayout.Get());
}

std::shared_ptr<InputLayout> InputLayout::Resolve(Graphics & gfx, const VertexLayout & layout, std::string vertexShaderName, ID3DBlob * pVertexShaderBytecode)
{
	return Bind::Codex::Resolve<InputLayout>(gfx, GenerateUID(layout, vertexShaderName, pVertexShaderBytecode), layout, vertexShaderName, pVertexShaderBytecode);
}

std::string InputLayout::GenerateUID(const VertexLayout & layout, std::string vertexShaderName, ID3DBlob * pVertexShaderBytecode)
{
	using namespace std::string_literals;
	return typeid(InputLayout).name() + "#"s + layout.GetCode() + "|" + vertexShaderName;
}
