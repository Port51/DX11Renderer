#pragma once
#include "BindingPass.h"

namespace Bind
{
	class IndexBuffer;
	class VertexBuffer;
	class VertexShader;
	class InputLayout;
}

namespace Rendergraph
{
	class FullscreenPass : public BindingPass
	{
	public:
		FullscreenPass(const std::string name, Graphics& gfx);
		void Execute(Graphics& gfx) const override;
	};
}