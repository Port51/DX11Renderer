#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Bindable.h"
#include "Binding.h"
#include "Graphics.h"

class MeshRenderer;
class FrameCommander;

class RenderStep
{
public:
	RenderStep(std::string _targetPass)
		: targetPass{ _targetPass }
	{}
	void AddBinding(std::shared_ptr<Bindable> pBindable, UINT slot = 0u)
	{
		bindings.push_back(Binding(std::move(pBindable), slot));
	}
	void AddBinding(Binding pBinding)
	{
		bindings.push_back(std::move(pBinding));
	}
	void SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const;
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