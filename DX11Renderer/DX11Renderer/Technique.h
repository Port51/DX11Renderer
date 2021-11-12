#pragma once
#include "Step.h"
#include <vector>
#include <memory>

class MeshRenderer;
class FrameCommander;

class Technique
{
public:
	Technique() = default;
	Technique(std::string name)
		:
		name(name)
	{}
	void SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const;
	void AddStep(std::unique_ptr<Step> step)
	{
		pSteps.push_back(std::move(step));
	}
	bool IsActive() const
	{
		return active;
	}
	void SetActiveState(bool active_in)
	{
		active = active_in;
	}
	void InitializeParentReferences(const MeshRenderer& parent);
	const std::string& GetName() const
	{
		return name;
	}
private:
	bool active = true;
	std::vector<std::unique_ptr<Step>> pSteps;
	std::string name = "Nameless Tech";
};