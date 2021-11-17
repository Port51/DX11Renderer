#pragma once
#include "RenderPass.h"
#include "Sink.h"

namespace Bind
{
	class Bindable;
	class DepthStencil;
	class RenderTarget;
}

namespace Rendergraph
{
	class BindingPass : public RenderPass
	{
	protected:
		BindingPass(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds = {});
		void AddBind(std::shared_ptr<Bind::Bindable> bind);
		void BindAll(Graphics& gfx) const;
		void Finalize() override;
	protected:
		template<class T>
		void AddBindSink(std::string name)
		{
			const auto index = binds.size();
			binds.emplace_back();
			RegisterSink(std::make_unique<ContainerBindableSink<T>>(std::move(name), binds, index));
		}
		std::shared_ptr<Bind::RenderTarget> renderTarget;
		std::shared_ptr<Bind::DepthStencil> depthStencil;
	private:
		void BindBufferResources(Graphics& gfx) const;
	private:
		std::vector<std::shared_ptr<Bind::Bindable>> binds;
	};
}