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
		NullPixelShader(const GraphicsDevice& gfx);
		virtual void Release() override;
	public:
		void BindPS(const GraphicsDevice& gfx, UINT slot) override;
		static std::shared_ptr<NullPixelShader> Resolve(const GraphicsDevice& gfx);
		static std::string GenerateUID();
	};
}