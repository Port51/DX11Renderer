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
	void NullPixelShader::BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_Shader, 0u))
		{
			gfx.GetContext()->PSSetShader(nullptr, nullptr, 0u);
		}
	}
	void NullPixelShader::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_Shader, 0u);
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