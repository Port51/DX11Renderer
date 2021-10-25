#include "Sampler.h"
#include "GraphicsThrowMacros.h"

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
{
	SETUP_LOGGING(gfx);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = wrapU;
	samplerDesc.AddressV = wrapV;
	samplerDesc.AddressW = wrapW;

	GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
}

Sampler::Sampler(Graphics& gfx, D3D11_SAMPLER_DESC samplerDesc)
{
	SETUP_LOGGING(gfx);
	GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
}

void Sampler::Bind(Graphics& gfx)
{
	// First argument = slot that shader will use
	GetContext(gfx)->PSSetSamplers(0u, 1u, pSampler.GetAddressOf());
}