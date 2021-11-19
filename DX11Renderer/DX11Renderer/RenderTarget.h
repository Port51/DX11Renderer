#pragma once
#include "Graphics.h"
#include "RenderTexture.h"

class DepthStencil;

class RenderTarget : public RenderTexture
{
public:
	RenderTarget(Graphics& gfx);
	bool Init(ID3D11Device* pDevice, int textureWidth, int textureHeight) override;
	void BindAsTexture(Graphics& gfx, UINT slot) const;
	void BindAsTarget(Graphics& gfx) const;
	void BindAsTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView) const;
private:
	D3D11_VIEWPORT viewport;
};