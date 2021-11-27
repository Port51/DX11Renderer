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
	FullscreenPass(Graphics& gfx, std::shared_ptr<Texture> pInput, const char* pixelShader);
	void Execute(Graphics& gfx) const override;
private:
	Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
	Binding& AddBinding(Binding pBinding);
	void SetupFullscreenQuadBindings(Graphics& gfx, std::string vertexShaderName, std::shared_ptr<VertexShader> vertexShader);
private:
	std::vector<Binding> bindings;
	std::unique_ptr<VertexBufferWrapper> pVertexBufferWrapper;
};