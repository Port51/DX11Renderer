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
		RasterizerState(const GraphicsDevice& gfx, const D3D11_CULL_MODE cullMode);
		virtual void Release() override;
		void BindRS(const GraphicsDevice& gfx, RenderState& renderState) override;
		void UnbindRS(const GraphicsDevice& gfx, RenderState& renderState) override;
		static std::shared_ptr<RasterizerState> Resolve(const GraphicsDevice& gfx, const D3D11_CULL_MODE cullMode);
		static const D3D11_CULL_MODE GetCullModeFromMaterialString(const std::string name);
	protected:
		static std::string GenerateUID(const D3D11_CULL_MODE cullMode);
	protected:
		ComPtr<ID3D11RasterizerState> m_pRasterizer;
		const D3D11_CULL_MODE m_cullMode;
	};
}