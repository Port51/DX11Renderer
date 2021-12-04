#pragma once
#include "Bindable.h"
#include "WindowsInclude.h"
#include <string>
#include <d3d11.h>
#include <wrl.h>

class Graphics;
struct ID3D11ShaderResourceView;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_SHADER_RESOURCE_VIEW_DESC;

class Texture : public Bindable
{
public:
	Texture(Graphics& gfx);
	Texture(Graphics& gfx, const std::string& path);
	Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc);
	Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
public:
	void BindCS(Graphics& gfx, UINT slot) override;
	void BindVS(Graphics& gfx, UINT slot) override;
	void BindPS(Graphics& gfx, UINT slot) override;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() const;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> GetUAV() const;
public:
	static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string& path);
protected:
	static std::string GenerateUID(const std::string& path);
protected:
	std::string path;
	UINT slot;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV;
};