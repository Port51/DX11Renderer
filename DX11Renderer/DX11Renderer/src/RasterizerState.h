#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <array>

class ID3D11RasterizerState;

namespace gfx
{
	class Graphics;

	class RasterizerState : public Bindable
	{
	public:
		RasterizerState(Graphics& gfx, D3D11_CULL_MODE cullMode);
		void BindRS(Graphics& gfx) override;
		static std::shared_ptr<RasterizerState> Resolve(Graphics& gfx, D3D11_CULL_MODE cullMode);
		static D3D11_CULL_MODE GetCullModeFromMaterialString(std::string name);
	protected:
		static std::string GenerateUID(D3D11_CULL_MODE cullMode);
	protected:
		ComPtr<ID3D11RasterizerState> pRasterizer;
		D3D11_CULL_MODE cullMode;
	};
}