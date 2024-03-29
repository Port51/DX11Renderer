#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

struct ID3D11ShaderResourceView;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_SUBRESOURCE_DATA;
struct D3D11_SHADER_RESOURCE_VIEW_DESC;

namespace gfx
{
	class GraphicsDevice;

	class Texture : public Bindable
	{
	public:
		enum class TextureDimension : u8 { TEXTURE1D, TEXTURE2D, TEXTURE3D };
	public:
		Texture(const GraphicsDevice& gfx);
		Texture(const GraphicsDevice& gfx, UINT mipCount);
		Texture(const GraphicsDevice& gfx, const std::string& path);
		Texture(const GraphicsDevice& gfx, const std::string& path, D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
		Texture(const GraphicsDevice& gfx, const D3D11_TEXTURE2D_DESC textureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
		virtual void Release() override;
	public:
		void BindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindCS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		const ComPtr<ID3D11ShaderResourceView>& GetSRV() const;
		const ComPtr<ID3D11UnorderedAccessView> GetUAV(const UINT mipSlice = 0u) const;
		const TextureDimension GetDimension() const;
	public:
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const std::string& path);
	protected:
		static std::string GenerateUID(const std::string& tag);
	protected:
		std::string m_tag;
		TextureDimension m_dimension;
		UINT m_mipCount;
		UINT m_slot;
		ComPtr<ID3D11Texture2D> m_pTexture;
		ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
		std::vector<ComPtr<ID3D11UnorderedAccessView>> m_pUAV;
	};
}