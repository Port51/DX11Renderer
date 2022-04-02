#include "pch.h"
#include "Sampler.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	Sampler::Sampler(GraphicsDevice& gfx)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(GraphicsDevice & gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
		: filter(filter), wrapU(wrapU), wrapV(wrapV), wrapW(wrapW)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = filter;
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

		THROW_IF_FAILED(gfx.GetAdapter()->CreateSamplerState(&samplerDesc, &pSampler));
	}

	Sampler::Sampler(GraphicsDevice& gfx, D3D11_SAMPLER_DESC samplerDesc)
	{
		THROW_IF_FAILED(gfx.GetAdapter()->CreateSamplerState(&samplerDesc, &pSampler));
	}

	void Sampler::BindCS(GraphicsDevice & gfx, UINT slot)
	{
		gfx.GetContext()->CSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	void Sampler::BindVS(GraphicsDevice & gfx, UINT slot)
	{
		gfx.GetContext()->VSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	void Sampler::BindPS(GraphicsDevice & gfx, UINT slot)
	{
		gfx.GetContext()->PSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	ComPtr<ID3D11SamplerState> Sampler::GetD3DSampler() const
	{
		return pSampler;
	}

	std::shared_ptr<Bindable> Sampler::Resolve(GraphicsDevice& gfx)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(GraphicsDevice & gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(GraphicsDevice & gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(GraphicsDevice & gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::move(Codex::Resolve<Sampler>(gfx, GenerateUID(filter, wrapU, wrapV, wrapW), wrapU, wrapV, wrapW));
	}

	std::string Sampler::GenerateUID(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::string(typeid(Sampler).name()) + "|"
			+ std::to_string(static_cast<uint32_t>(filter))
			+ std::to_string(static_cast<uint32_t>(wrapU))
			+ std::to_string(static_cast<uint32_t>(wrapV))
			+ std::to_string(static_cast<uint32_t>(wrapW));
	}
}