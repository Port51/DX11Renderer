#pragma once
#include "Shader.h"
#include "CommonHeader.h"
#include <string>

namespace gfx
{
	class GraphicsDevice;

	class NullPixelShader : public Shader
	{
	public:
		NullPixelShader(const GraphicsDevice& gfx);
		virtual void Release() override;
	public:
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		static std::shared_ptr<NullPixelShader> Resolve(const GraphicsDevice& gfx);
		static std::string GenerateUID();
	};
}