#pragma once
#include "Bindable.h"

class Texture : public Bindable
{
public:
	Texture(Graphics& gfx, const class Surface& s);
	Texture(Graphics& gfx, const class Surface& s, D3D11_TEXTURE2D_DESC textureDesc);
	Texture(Graphics& gfx, const class Surface& s, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
	void Bind(Graphics& gfx) override;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
};