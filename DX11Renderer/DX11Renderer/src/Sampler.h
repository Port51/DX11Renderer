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
		Sampler(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
		Sampler(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
		Sampler(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(const GraphicsDevice& gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(const GraphicsDevice& gfx, D3D11_SAMPLER_DESC samplerDesc);
		virtual void Release() override;
	public:
		void BindCS(const GraphicsDevice& gfx, UINT slot) override;
		void BindVS(const GraphicsDevice& gfx, UINT slot) override;
		void BindPS(const GraphicsDevice& gfx, UINT slot) override;
		const ComPtr<ID3D11SamplerState> GetD3DSampler() const;
	public:
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::string GenerateUID(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
	protected:
		ComPtr<ID3D11SamplerState> m_pSampler;
		D3D11_FILTER m_filter;
		D3D11_TEXTURE_ADDRESS_MODE m_wrapU;
		D3D11_TEXTURE_ADDRESS_MODE m_wrapV;
		D3D11_TEXTURE_ADDRESS_MODE m_wrapW;
	};
}