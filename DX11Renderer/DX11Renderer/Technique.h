#pragma once
#include "RenderStep.h"
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
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, const MeshRenderer& renderer) const;
	void AddStep(std::unique_ptr<RenderStep> step)
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
	std::vector<std::unique_ptr<RenderStep>> pSteps;
	std::string name = "Nameless Tech";
};