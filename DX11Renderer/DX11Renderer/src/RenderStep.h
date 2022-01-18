#pragma once
#include "CommonHeader.h"
#include <vector>
#include <string>

namespace gfx
{
	class Graphics;
	class MeshRenderer;
	class Renderer;
	class RenderPass;
	class Binding;
	class Bindable;
	struct DrawContext;

	class RenderStep
	{
	public:
		RenderStep(std::string _targetPass);
		virtual ~RenderStep() = default;
	public:
		Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
		Binding& AddBinding(Binding pBinding);
		void SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
		void Bind(Graphics& gfx) const;
	private:
		std::string targetPass;
		std::vector<Binding> bindings;
	};
}