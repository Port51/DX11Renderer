#pragma once
#include "BindingPass.h"

namespace Bind
{
	class IndexBuffer;
	class VertexBuffer;
	class VertexShader;
	class InputLayout;
}

class FullscreenPass : public RenderPass
{
public:
	FullscreenPass(const std::string name, Graphics& gfx);
	void Execute(Graphics& gfx) const override;
};