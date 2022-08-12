#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "DX11Include.h"
#include <string>

namespace gfx
{
	class GraphicsDevice;

	class Sampler : public Bindable
	{
	public:
		Sampler(const GraphicsDevice& gfx);
		Sampler(const GraphicsDevice& gfx, const D3D11_TEXTURE_ADDRESS_MODE wrapU);
		Sampler(const GraphicsDevice& gfx, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV);
		Sampler(const GraphicsDevice& gfx, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV, const D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(const GraphicsDevice& gfx, const D3D11_FILTER filter, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV, const D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(const GraphicsDevice& gfx, const D3D11_SAMPLER_DESC samplerDesc);
		virtual void Release() override;
	public:
		void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		const ComPtr<ID3D11SamplerState>& GetD3DSampler() const;
	public:
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const D3D11_TEXTURE_ADDRESS_MODE wrapU);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV, const D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const D3D11_FILTER filter, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV, const D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::string GenerateUID(const D3D11_FILTER filter, const D3D11_TEXTURE_ADDRESS_MODE wrapU, const D3D11_TEXTURE_ADDRESS_MODE wrapV, const D3D11_TEXTURE_ADDRESS_MODE wrapW);
	protected:
		ComPtr<ID3D11SamplerState> m_pSampler;
		const D3D11_FILTER m_filter;
		const D3D11_TEXTURE_ADDRESS_MODE m_wrapU;
		const D3D11_TEXTURE_ADDRESS_MODE m_wrapV;
		const D3D11_TEXTURE_ADDRESS_MODE m_wrapW;
	};
}