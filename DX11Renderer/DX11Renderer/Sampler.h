#pragma once
#include "Bindable.h"
#include <wrl.h>
#include <d3d11.h>

class Graphics;

class Sampler : public Bindable
{
public:
	Sampler(Graphics& gfx);
	Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU);
	Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV);
	Sampler(Graphics& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW);
	Sampler(Graphics& gfx, D3D11_SAMPLER_DESC samplerDesc);
	void Bind(Graphics& gfx) override;
	std::string GetUID() const override;
public:
	static std::shared_ptr<Bindable> Resolve(Graphics& gfx);
	static std::string GenerateUID();
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
};