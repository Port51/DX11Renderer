#pragma once
#include "CommonHeader.h"
#include <vector>
#include <string>

namespace gfx
{
	class GraphicsDevice;
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
		void Bind(const GraphicsDevice& gfx) const;
	private:
		std::string m_targetPass;
		std::vector<Binding> m_bindings;
	};
}