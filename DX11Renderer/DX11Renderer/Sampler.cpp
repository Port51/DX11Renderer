#include "Sampler.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	Sampler::Sampler(Graphics& gfx, Type type, bool reflect)
		: Sampler(gfx, type, reflect, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(Graphics& gfx, Type type, bool reflect, D3D11_TEXTURE_ADDRESS_MODE wrapU)
		: Sampler(gfx, type, reflect, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(Graphics& gfx, Type type, bool reflect, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
		: Sampler(gfx, type, reflect, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(Graphics& gfx, Type type, bool reflect, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
		: type(type),
		reflect(reflect)
	{
		SETUP_LOGGING(gfx);

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = wrapU;
		samplerDesc.AddressV = wrapV;
		samplerDesc.AddressW = wrapW;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
	}

	Sampler::Sampler(Graphics& gfx, Type type, bool reflect, D3D11_SAMPLER_DESC samplerDesc)
		: type(type),
		reflect(reflect)
	{
		SETUP_LOGGING(gfx);
		GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
	}

	void Sampler::Bind(Graphics& gfx, UINT slot)
	{
		// First argument = slot that shader will use
		GetContext(gfx)->PSSetSamplers(slot, 1u, pSampler.GetAddressOf());
	}

	std::string Sampler::GetUID() const
	{
		return GenerateUID();
	}

	std::shared_ptr<Bindable> Sampler::Resolve(Graphics & gfx)
	{
		return Bind::Codex::Resolve<Sampler>(gfx);
	}

	std::string Sampler::GenerateUID()
	{
		return typeid(Sampler).name();
	}
}