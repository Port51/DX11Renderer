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
	void BindPS(Graphics& gfx, const RenderPass& renderPass, UINT slot) override;
public:
	static std::shared_ptr<PixelShader> Resolve(Graphics& gfx, const std::string& path);
	static std::string GenerateUID(const std::string& path);
protected:
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
};