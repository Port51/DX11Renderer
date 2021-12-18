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

	virtual void Init(ID3D11Device* pDevice, int textureWidth, int textureHeight);
	void Shutdown();

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetView() const;

	void BindCS(Graphics& gfx, UINT slot) override;
	void BindVS(Graphics& gfx, UINT slot) override;
	void BindPS(Graphics& gfx, UINT slot) override;

protected:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView;
};