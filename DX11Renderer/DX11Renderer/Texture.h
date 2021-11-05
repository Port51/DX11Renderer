#pragma once
#include "Bindable.h"

class Texture : public Bindable
{
public:
	Texture(Graphics& gfx, const std::string& path, UINT slot);
	Texture(Graphics& gfx, const std::string& path, UINT slot, D3D11_TEXTURE2D_DESC textureDesc);
	Texture(Graphics& gfx, const std::string& path, UINT slot, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
	void Bind(Graphics& gfx) override;
	std::string GetUID() const override;
public:
	static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string& path, UINT slot);
	static std::string GenerateUID(const std::string& path, UINT slot);
protected:
	std::string path;
	UINT slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
};