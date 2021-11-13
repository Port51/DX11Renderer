#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include <wrl.h>

class Graphics;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

namespace Bind
{
	class RenderTarget;

	class DepthStencil : public Bindable, public BufferResource
	{
		friend class RenderTarget;
	public:
		void BindAsBuffer(Graphics& gfx) override;
		void BindAsBuffer(Graphics& gfx, BufferResource* renderTarget) override;
		void BindAsBuffer(Graphics& gfx, RenderTarget* rt);
		void Clear(Graphics& gfx) override;
	protected:
		DepthStencil(Graphics& gfx, UINT width, UINT height, bool canBindShaderInput);
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	};

	class ShaderInputDepthStencil : public DepthStencil
	{
	public:
		ShaderInputDepthStencil(Graphics& gfx, UINT slot);
		ShaderInputDepthStencil(Graphics& gfx, UINT width, UINT height, UINT slot);
		void Bind(Graphics& gfx, UINT slot) override;
	private:
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	};

	class OutputOnlyDepthStencil : public DepthStencil
	{
	public:
		OutputOnlyDepthStencil(Graphics& gfx);
		OutputOnlyDepthStencil(Graphics& gfx, UINT width, UINT height);
		void Bind(Graphics& gfx, UINT slot) override;
	};
}