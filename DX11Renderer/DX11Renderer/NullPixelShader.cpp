#include "NullPixelShader.h"
#include "GraphicsThrowMacros.h"
#include "SharedCodex.h"
#include "ChiliUtil.h"

namespace Bind
{
	NullPixelShader::NullPixelShader(Graphics& gfx)
	{
	}
	void NullPixelShader::BindPS(Graphics& gfx, const RenderPass& renderPass, UINT slot)
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