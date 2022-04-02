#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>

struct ID3D11PixelShader;

namespace gfx
{
	class GraphicsDevice;

	class PixelShader : public Bindable
	{
	public:
		PixelShader(GraphicsDevice& gfx, const std::string& path);
		void BindPS(GraphicsDevice& gfx, UINT slot) override;
	public:
		static std::shared_ptr<PixelShader> Resolve(GraphicsDevice& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
	protected:
		std::string path;
		ComPtr<ID3D11PixelShader> pPixelShader;
	};
}