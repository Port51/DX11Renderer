#pragma once
#include "Bindable.h"
#include "WindowsInclude.h"
#include <string>
#include <d3d11.h>
#include <wrl.h>

class Graphics;

namespace Bind
{
	class Texture : public Bindable
	{
	public:
		Texture(Graphics& gfx, const std::string& path);
		Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc);
		Texture(Graphics& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
		void Bind(Graphics& gfx, UINT slot) override;
		std::string GetUID() const override;
	public:
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
	protected:
		std::string path;
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
	};
}