#include "InputLayout.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include "VertexLayout.h"

namespace Bind
{
	InputLayout::InputLayout(Graphics& gfx, VertexLayout _layout, ID3DBlob* pVertexShaderBytecode)
		: layout(std::move(_layout))
	{
		SETUP_LOGGING(gfx);

		const auto d3dLayout = layout.GetD3DLayout();
		GFX_THROW_INFO(GetDevice(gfx)->CreateInputLayout(
			d3dLayout.data(), (UINT)d3dLayout.size(),
			pVertexShaderBytecode->GetBufferPointer(),
			pVertexShaderBytecode->GetBufferSize(),
			&pInputLayout
		));
	}

	void InputLayout::Bind(Graphics& gfx, UINT slot)
	{
		GetContext(gfx)->IASetInputLayout(pInputLayout.Get());
	}

	std::string InputLayout::GetUID() const
	{
		return GenerateUID(layout);
	}

	std::shared_ptr<InputLayout> InputLayout::Resolve(Graphics & gfx, const VertexLayout & layout, ID3DBlob * pVertexShaderBytecode)
	{
		return Bind::Codex::Resolve<InputLayout>(gfx, layout, pVertexShaderBytecode);
	}

	std::string InputLayout::GenerateUID(const VertexLayout & layout, ID3DBlob * pVertexShaderBytecode)
	{
		using namespace std::string_literals;
		return typeid(InputLayout).name() + "#"s + layout.GetCode();
	}
}