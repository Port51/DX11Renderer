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
		FullscreenPass(const GraphicsDevice& gfx, std::string name, const char* pixelShader);
	public:
		void Execute(const GraphicsDevice& gfx) const override;
		void SetInputTarget(std::shared_ptr<Texture> pInput);
	private:
		void SetupFullscreenQuadBindings(const GraphicsDevice& gfx, std::string vertexShaderName, const VertexShader& vertexShader);
	private:
		std::shared_ptr<Texture> m_pInputTexture;
		std::unique_ptr<VertexBufferWrapper> m_pVertexBufferWrapper;
	};
}