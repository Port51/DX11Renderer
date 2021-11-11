#pragma once
#include "Step.h"
#include <vector>


class Technique
{
public:
	Technique() = default;
	Technique(std::string name) noexcept
		:
		name(name)
	{}
	void SubmitDrawCalls(class FrameCommander& frame, const class MeshRenderer& renderer) const noexcept;
	void AddStep(Step step) noexcept
	{
		steps.push_back(std::move(step));
	}
	bool IsActive() const noexcept
	{
		return active;
	}
	void SetActiveState(bool active_in) noexcept
	{
		active = active_in;
	}
	void InitializeParentReferences(const class MeshRenderer& parent) noexcept;
	const std::string& GetName() const noexcept
	{
		return name;
	}
private:
	bool active = true;
	std::vector<Step> steps;
	std::string name = "Nameless Tech";
};