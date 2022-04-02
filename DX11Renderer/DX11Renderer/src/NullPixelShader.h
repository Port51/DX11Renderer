#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>

namespace gfx
{
	class GraphicsDevice;

	class NullPixelShader : public Bindable
	{
	public:
		NullPixelShader(GraphicsDevice& gfx);
	public:
		void BindPS(GraphicsDevice& gfx, UINT slot) override;
		static std::shared_ptr<NullPixelShader> Resolve(GraphicsDevice& gfx);
		static std::string GenerateUID();
	};
}