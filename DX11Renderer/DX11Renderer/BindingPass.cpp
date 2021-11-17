#include "BindingPass.h"
#include "Bindable.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "RenderGraphCompileException.h"

namespace Rendergraph
{
	BindingPass::BindingPass(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds)
		:
		RenderPass(std::move(name)),
		binds(std::move(binds))
	{}

	void BindingPass::AddBind(std::shared_ptr<Bind::Bindable> bind)
	{
		binds.push_back(std::move(bind));
	}

	void BindingPass::BindAll(Graphics& gfx) const
	{
		BindBufferResources(gfx);
		for (auto& bind : binds)
		{
			bind->Bind(gfx, 0u);
		}
	}

	void BindingPass::Finalize()
	{
		RenderPass::Finalize();
		if (!renderTarget && !depthStencil)
		{
			throw RGC_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void BindingPass::BindBufferResources(Graphics& gfx) const
	{
		if (renderTarget)
		{
			renderTarget->BindAsBuffer(gfx, depthStencil.get());
		}
		else
		{
			depthStencil->BindAsBuffer(gfx);
		}
	}
}