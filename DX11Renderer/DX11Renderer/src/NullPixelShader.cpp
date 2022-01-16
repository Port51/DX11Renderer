#include "pch.h"
#include "NullPixelShader.h"
#include "SharedCodex.h"
#include "Graphics.h"

namespace gfx
{
	NullPixelShader::NullPixelShader(Graphics& gfx)
	{
	}
	void NullPixelShader::BindPS(Graphics& gfx, UINT slot)
	{
		gfx.GetContext()->PSSetShader(nullptr, nullptr, 0u);
	}
	std::shared_ptr<NullPixelShader> NullPixelShader::Resolve(Graphics& gfx)
	{
		return Codex::Resolve<NullPixelShader>(gfx, GenerateUID());
	}
	std::string NullPixelShader::GenerateUID()
	{
		return typeid(NullPixelShader).name();
	}
}