#pragma once
#include "Bindable.h"
#include <string>
#include <wrl.h>

class Graphics;
struct ID3D11PixelShader;

class PixelShader : public Bindable
{
public:
	PixelShader(Graphics& gfx, const std::string& path);
	void Bind(Graphics& gfx, UINT slot) override;
	std::string GetUID() const noexcept override;
public:
	static std::shared_ptr<PixelShader> Resolve(Graphics& gfx, const std::string& path);
	static std::string GenerateUID(const std::string& path);
protected:
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
};