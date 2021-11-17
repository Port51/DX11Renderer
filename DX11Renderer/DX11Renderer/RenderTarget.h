#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include <wrl.h>

class Graphics;
class DepthStencil;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

namespace Bind
{

	class RenderTarget : public Bindable, public BufferResource
	{
	public:
		void BindAsBuffer(Graphics& gfx) override;
		void BindAsBuffer(Graphics& gfx, BufferResource* depthStencil) override;
		void BindAsBuffer(Graphics& gfx, DepthStencil* depthStencil);
		void Clear(Graphics& gfx) override;
		void Clear(Graphics& gfx, const std::array<float, 4>& color);
		UINT GetWidth() const;
		UINT GetHeight() const;
	private:
		void BindAsBuffer(Graphics& gfx, ID3D11DepthStencilView* pDepthStencilView);
	protected:
		RenderTarget(Graphics& gfx, ID3D11Texture2D* pTexture);
		RenderTarget(Graphics& gfx, UINT width, UINT height);
		UINT width;
		UINT height;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetView;
	};

	class ShaderInputRenderTarget : public RenderTarget
	{
	public:
		ShaderInputRenderTarget(Graphics& gfx, UINT width, UINT height, UINT slot);
		void Bind(Graphics& gfx, UINT slot) override;
	private:
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	};

	// RT for Graphics to create RenderTarget for the back buffer
	class OutputOnlyRenderTarget : public RenderTarget
	{
		friend Graphics;
	public:
		void Bind(Graphics& gfx, UINT slot) override;
	private:
		OutputOnlyRenderTarget(Graphics& gfx, ID3D11Texture2D* pTexture);
	};
}