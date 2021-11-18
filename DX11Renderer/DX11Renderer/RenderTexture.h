#pragma once
#include <d3d11.h>
#include "Texture.h"

class RenderTexture : Texture
{
public:
	RenderTexture(Graphics& gfx);
	~RenderTexture();

	bool Init(ID3D11Device* pDevice, int textureWidth, int textureHeight);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float, float, float, float);

private:
	ID3D11Texture2D* pRenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView;
};