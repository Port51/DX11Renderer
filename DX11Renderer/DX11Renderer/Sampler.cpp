#include "Sampler.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

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

Sampler::Sampler(Graphics& gfx, D3D11_SAMPLER_DESC samplerDesc)
{
	SETUP_LOGGING(gfx);
	GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
}

void Sampler::BindCS(Graphics & gfx, UINT slot)
{
	GetContext(gfx)->CSSetSamplers(slot, 1u, pSampler.GetAddressOf());
}

void Sampler::BindVS(Graphics & gfx, UINT slot)
{
	GetContext(gfx)->VSSetSamplers(slot, 1u, pSampler.GetAddressOf());
}

void Sampler::BindPS(Graphics & gfx, UINT slot)
{
	GetContext(gfx)->PSSetSamplers(slot, 1u, pSampler.GetAddressOf());
}

std::string Sampler::GetUID() const
{
	return GenerateUID(wrapU, wrapV, wrapW);
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
	return Bind::Codex::Resolve<Sampler>(gfx, wrapU, wrapV, wrapW);
}

std::string Sampler::GenerateUID()
{
	return GenerateUID(D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP);
}

std::string Sampler::GenerateUID(D3D11_TEXTURE_ADDRESS_MODE wrapU)
{
	return GenerateUID(wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP);
}

std::string Sampler::GenerateUID(D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
{
	return GenerateUID(wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP);
}

std::string Sampler::GenerateUID(D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
{
	return std::string(typeid(Sampler).name()) + "|"
		+ std::to_string(static_cast<uint32_t>(wrapU))
		+ std::to_string(static_cast<uint32_t>(wrapV))
		+ std::to_string(static_cast<uint32_t>(wrapW));
}
