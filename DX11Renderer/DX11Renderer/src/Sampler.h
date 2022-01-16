#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "DX11Include.h"
#include <string>

namespace gfx
{
	class Graphics;

	class Sampler : public Bindable
	{
	public:
		Sampler(Graphics& gfx);
		Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
		Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
		Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		Sampler(Graphics& gfx, D3D11_SAMPLER_DESC samplerDesc);
	public:
		void BindCS(Graphics& gfx, UINT slot) override;
		void BindVS(Graphics& gfx, UINT slot) override;
		void BindPS(Graphics& gfx, UINT slot) override;
	public:
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx);
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
		static std::string GenerateUID(D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
	protected:
		ComPtr<ID3D11SamplerState> pSampler;
		D3D11_TEXTURE_ADDRESS_MODE wrapU;
		D3D11_TEXTURE_ADDRESS_MODE wrapV;
		D3D11_TEXTURE_ADDRESS_MODE wrapW;
	};
}