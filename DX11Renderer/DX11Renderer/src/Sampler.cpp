#include "pch.h"
#include "Sampler.h"
#include "SharedCodex.h"
#include "Graphics.h"

namespace gfx
{
	Sampler::Sampler(Graphics& gfx)
		: Sampler(gfx, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU)
		: Sampler(gfx, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
		: Sampler(gfx, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
		: wrapU(wrapU), wrapV(wrapV), wrapW(wrapW)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = wrapU;
		samplerDesc.AddressV = wrapV;
		samplerDesc.AddressW = wrapW;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		// todo: add color option in constructor
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;

		THROW_IF_FAILED(gfx.GetDevice()->CreateSamplerState(&samplerDesc, &pSampler));
	}

	Sampler::Sampler(Graphics& gfx, D3D11_SAMPLER_DESC samplerDesc)
	{
		THROW_IF_FAILED(gfx.GetDevice()->CreateSamplerState(&samplerDesc, &pSampler));
	}

	void Sampler::BindCS(Graphics & gfx, UINT slot)
	{
		gfx.GetContext()->CSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	void Sampler::BindVS(Graphics & gfx, UINT slot)
	{
		gfx.GetContext()->VSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	void Sampler::BindPS(Graphics & gfx, UINT slot)
	{
		gfx.GetContext()->PSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	std::shared_ptr<Bindable> Sampler::Resolve(Graphics& gfx)
	{
		return Resolve(gfx, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP);
	}

	std::shared_ptr<Bindable> Sampler::Resolve(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU)
	{
		return Resolve(gfx, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP);
	}

	std::shared_ptr<Bindable> Sampler::Resolve(Graphics & gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
	{
		return Resolve(gfx, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP);
	}

	std::shared_ptr<Bindable> Sampler::Resolve(Graphics & gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return Codex::Resolve<Sampler>(gfx, GenerateUID(wrapU, wrapV, wrapW), wrapU, wrapV, wrapW);
	}

	std::string Sampler::GenerateUID(D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::string(typeid(Sampler).name()) + "|"
			+ std::to_string(static_cast<uint32_t>(wrapU))
			+ std::to_string(static_cast<uint32_t>(wrapV))
			+ std::to_string(static_cast<uint32_t>(wrapW));
	}
}