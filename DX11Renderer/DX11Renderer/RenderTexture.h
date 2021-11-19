#pragma once
#include "Texture.h"
#include <d3d11.h>
#include <wrl.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;

class RenderTexture : public Texture
{
public:
	RenderTexture(Graphics& gfx);
	//~RenderTexture();

	virtual bool Init(ID3D11Device* pDevice, int textureWidth, int textureHeight);
	void Shutdown();

	void Bind(Graphics& gfx, UINT slot) override; // todo: don't override this

public:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pRenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pRenderTextureView;
};