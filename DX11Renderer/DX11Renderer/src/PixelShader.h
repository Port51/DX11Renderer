#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>

struct ID3D11PixelShader;

namespace gfx
{
	class Graphics;

	class PixelShader : public Bindable
	{
	public:
		PixelShader(Graphics& gfx, const std::string& path);
		void BindPS(Graphics& gfx, UINT slot) override;
	public:
		static std::shared_ptr<PixelShader> Resolve(Graphics& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
	protected:
		std::string path;
		ComPtr<ID3D11PixelShader> pPixelShader;
	};
}