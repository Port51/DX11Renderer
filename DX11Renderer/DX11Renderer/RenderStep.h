#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Bindable.h"
#include "Binding.h"

class MeshRenderer;
class Graphics;

namespace Rgph
{
	class FrameCommander;

	class RenderStep
	{
	public:
		RenderStep(std::string _targetPass)
			: targetPass{ _targetPass }
		{}
		void AddBinding(std::shared_ptr<Bind::Bindable> pBindable, UINT slot = 0u);
		void AddBinding(Bind::Binding pBinding);
		void SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const;
		void Bind(Graphics& gfx) const;
		void InitializeParentReferences(const MeshRenderer& parent);
	private:
		std::string targetPass;
		std::vector<Bind::Binding> bindings;
	};
}