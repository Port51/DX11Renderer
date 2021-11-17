#pragma once
#include "Graphics.h"
#include "GraphicsResource.h"


class DepthStencil : public GraphicsResource
{
	friend class RenderTarget;
	friend class Graphics;
public:
	DepthStencil(Graphics& gfx, UINT width, UINT height);
	void BindAsDepthStencil(Graphics& gfx) const;
	void Clear(Graphics& gfx) const;
private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
};