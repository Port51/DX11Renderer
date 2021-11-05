#pragma once
#include "Bindable.h"

class PixelShader : public Bindable
{
public:
	PixelShader(Graphics& gfx, const std::string& path);
	void Bind(Graphics& gfx) override;
	std::string GetUID() const noexcept override;
public:
	static std::shared_ptr<PixelShader> Resolve(Graphics& gfx, const std::string& path);
	static std::string GenerateUID(const std::string& path);
protected:
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
};