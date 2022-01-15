#pragma once
#include "DX11Include.h"
#include "CommonHeader.h"

class Graphics;

class DepthStencilTarget
{
public:
	DepthStencilTarget(Graphics& gfx, int width, int height);
	void Clear(Graphics& gfx);
	ComPtr<ID3D11DepthStencilView> GetView() const;
	ComPtr<ID3D11ShaderResourceView> GetSRV() const;
private:
	ComPtr<ID3D11Texture2D> pDepthStencil;
	ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
};