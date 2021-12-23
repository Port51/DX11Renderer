#pragma once
#include "RenderPass.h"
#include <vector>
#include "Binding.h"
#include "Bindable.h"

class Binding;
class Bindable;
class VertexBufferWrapper;
class Texture;
class VertexShader;

class FullscreenPass : public RenderPass
{
public:
	FullscreenPass(Graphics& gfx, std::string name, const char* pixelShader);
public:
	void Execute(Graphics& gfx) const override;
	void SetInputTarget(std::shared_ptr<Texture> pInput);
private:
	Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
	Binding& AddBinding(Binding pBinding);
	void SetupFullscreenQuadBindings(Graphics& gfx, std::string vertexShaderName, std::shared_ptr<VertexShader> vertexShader);
private:
	std::vector<Binding> bindings;
	std::shared_ptr<Texture> pInputTexture;
	std::unique_ptr<VertexBufferWrapper> pVertexBufferWrapper;
};