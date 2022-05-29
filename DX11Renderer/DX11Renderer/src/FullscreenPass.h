#pragma once
#include "RenderPass.h"
#include <vector>

namespace gfx
{
	class Binding;
	class Bindable;
	class VertexBufferWrapper;
	class Texture;
	class VertexShader;

	class FullscreenPass : public RenderPass
	{
	public:
		FullscreenPass(const GraphicsDevice& gfx, const RenderPassType renderPassType, const char* pixelShader);
	public:
		void Execute(const GraphicsDevice& gfx) const override;
		void SetInputTarget(ComPtr<ID3D11ShaderResourceView> pInput);
	private:
		void SetupFullscreenQuadBindings(const GraphicsDevice& gfx, const char* vertexShaderName, const VertexShader& vertexShader);
	private:
		ComPtr<ID3D11ShaderResourceView> m_pInputTexture;
		std::unique_ptr<VertexBufferWrapper> m_pVertexBufferWrapper;
	};
}