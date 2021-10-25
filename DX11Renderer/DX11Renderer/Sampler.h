#pragma once
#include "Bindable.h"

class Sampler : public Bindable
{
public:
	Sampler(Graphics& gfx);
	Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
	Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
	Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
	Sampler(Graphics& gfx, D3D11_SAMPLER_DESC samplerDesc);
	void Bind(Graphics& gfx) override;
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
};