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
		FullscreenPass(GraphicsDevice& gfx, std::string name, const char* pixelShader);
	public:
		void Execute(GraphicsDevice& gfx) const override;
		void SetInputTarget(std::shared_ptr<Texture> pInput);
	private:
		void SetupFullscreenQuadBindings(GraphicsDevice& gfx, std::string vertexShaderName, std::shared_ptr<VertexShader> vertexShader);
	private:
		std::shared_ptr<Texture> pInputTexture;
		std::unique_ptr<VertexBufferWrapper> pVertexBufferWrapper;
	};
}