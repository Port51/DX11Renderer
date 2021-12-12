#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Bindable.h"
#include "Binding.h"
#include "Graphics.h"

class MeshRenderer;
class Renderer;
class RenderPass;

class RenderStep
{
public:
	RenderStep(std::string _targetPass)
		: targetPass{ _targetPass }
	{}
	Binding& AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		bindings.push_back(Binding(std::move(pBindable)));
		return bindings[bindings.size() - 1];
	}
	Binding& AddBinding(Binding pBinding)
	{
		bindings.push_back(std::move(pBinding));
		return bindings[bindings.size() - 1];
	}
	void SubmitDrawCalls(std::unique_ptr<Renderer>& frame, const MeshRenderer& renderer) const;
	void Bind(Graphics& gfx) const
	{
		for (const auto& b : bindings)
		{
			b.Bind(gfx);
		}
	}
	void InitializeParentReferences(const MeshRenderer& parent);
private:
	std::string targetPass;
	std::vector<Binding> bindings;
};