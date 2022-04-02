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
		Sampler(GraphicsDevice& gfx);
		Sampler(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
		Sampler(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
		Sampler(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(GraphicsDevice& gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(GraphicsDevice& gfx, D3D11_SAMPLER_DESC samplerDesc);
	public:
		void BindCS(GraphicsDevice& gfx, UINT slot) override;
		void BindVS(GraphicsDevice& gfx, UINT slot) override;
		void BindPS(GraphicsDevice& gfx, UINT slot) override;
		ComPtr<ID3D11SamplerState> GetD3DSampler() const;
	public:
		static std::shared_ptr<Bindable> Resolve(GraphicsDevice& gfx);
		static std::shared_ptr<Bindable> Resolve(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
		static std::shared_ptr<Bindable> Resolve(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
		static std::shared_ptr<Bindable> Resolve(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::shared_ptr<Bindable> Resolve(GraphicsDevice& gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::string GenerateUID(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
	protected:
		ComPtr<ID3D11SamplerState> pSampler;
		D3D11_FILTER filter;
		D3D11_TEXTURE_ADDRESS_MODE wrapU;
		D3D11_TEXTURE_ADDRESS_MODE wrapV;
		D3D11_TEXTURE_ADDRESS_MODE wrapW;
	};
}