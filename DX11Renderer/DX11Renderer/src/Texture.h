#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <wrl.h>
#include <string>
#include "DX11Include.h"

struct ID3D11ShaderResourceView;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_SHADER_RESOURCE_VIEW_DESC;

namespace gfx
{
	class Graphics;

	class Texture : public Bindable
	{
	public:
		Texture(Graphics& gfx);
		Texture(Graphics& gfx, const std::string& path);
		Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc);
		Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
	public:
		void BindCS(Graphics& gfx, UINT slot) override;
		void BindVS(Graphics& gfx, UINT slot) override;
		void BindPS(Graphics& gfx, UINT slot) override;
		ComPtr<ID3D11ShaderResourceView> GetSRV() const;
		ComPtr<ID3D11UnorderedAccessView> GetUAV() const;
	public:
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string& path);
	protected:
		static std::string GenerateUID(const std::string& path);
	protected:
		std::string path;
		UINT slot;
		ComPtr<ID3D11Texture2D> pTexture;
		ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
		ComPtr<ID3D11UnorderedAccessView> pUAV;
	};
}