#pragma once
#include "RenderPass.h"

namespace Bind
{
	class BufferResource;
}

namespace Rendergraph
{
	class BufferClearPass : public RenderPass
	{
	public:
		BufferClearPass(std::string name);
		void Execute(Graphics& gfx) const override;
	private:
		std::shared_ptr<Bind::BufferResource> buffer;
	};
}