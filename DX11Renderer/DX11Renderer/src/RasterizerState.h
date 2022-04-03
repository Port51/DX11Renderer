#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <array>

struct ID3D11RasterizerState;

namespace gfx
{
	class GraphicsDevice;

	class RasterizerState : public Bindable
	{
	public:
		RasterizerState(GraphicsDevice& gfx, D3D11_CULL_MODE cullMode);
		void BindRS(GraphicsDevice& gfx) override;
		static std::shared_ptr<RasterizerState> Resolve(GraphicsDevice& gfx, D3D11_CULL_MODE cullMode);
		static D3D11_CULL_MODE GetCullModeFromMaterialString(std::string name);
	protected:
		static std::string GenerateUID(D3D11_CULL_MODE cullMode);
	protected:
		ComPtr<ID3D11RasterizerState> m_pRasterizer;
		D3D11_CULL_MODE m_cullMode;
	};
}