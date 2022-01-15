#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>

class Graphics;

namespace Bind
{
	class NullPixelShader : public Bindable
	{
	public:
		NullPixelShader(Graphics& gfx);
	public:
		void BindPS(Graphics& gfx, UINT slot) override;
		static std::shared_ptr<NullPixelShader> Resolve(Graphics& gfx);
		static std::string GenerateUID();
	};
}