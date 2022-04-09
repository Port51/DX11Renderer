#include "pch.h"
#include "NullPixelShader.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	NullPixelShader::NullPixelShader(const GraphicsDevice& gfx)
	{
	}
	void NullPixelShader::Release()
	{
	}
	void NullPixelShader::BindPS(const GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->PSSetShader(nullptr, nullptr, 0u);
	}
	std::shared_ptr<NullPixelShader> NullPixelShader::Resolve(const GraphicsDevice& gfx)
	{
		return std::move(Codex::Resolve<NullPixelShader>(gfx, GenerateUID()));
	}
	std::string NullPixelShader::GenerateUID()
	{
		return typeid(NullPixelShader).name();
	}
}