#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Bindable.h"
#include "Graphics.h"

class MeshRenderer;
class FrameCommander;

class Step
{
public:
	Step(std::string _targetPass)
		: targetPass{ _targetPass }
	{}
	void AddBindable(std::shared_ptr<Bindable> bind_in) noexcept
	{
		pBindables.push_back(std::move(bind_in));
	}
	void SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const;
	void Bind(Graphics& gfx) const
	{
		for (const auto& b : pBindables)
		{
			b->Bind(gfx);
		}
	}
	void InitializeParentReferences(const MeshRenderer& parent) noexcept;
private:
	std::string targetPass;
	std::vector<std::shared_ptr<Bindable>> pBindables;
};