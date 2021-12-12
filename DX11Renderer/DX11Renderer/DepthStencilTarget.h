#pragma once
#include <d3d11.h>
#include <wrl.h>

class Graphics;

class DepthStencilTarget
{
public:
	DepthStencilTarget(Graphics& gfx, int width, int height);
	void Clear(Graphics& gfx);
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetView() const;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() const;
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
};