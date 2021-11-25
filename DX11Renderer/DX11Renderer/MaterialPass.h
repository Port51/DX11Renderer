#pragma once
#include <memory>

class Technique;
class MeshRenderer;
class FrameCommander;

///
/// Collection of bindables needed to execute a material pass in a technique
///
class MaterialPass
{
public:
	MaterialPass();
public:
	void AddTechnique(std::unique_ptr<Technique> _pTechnique);
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, const MeshRenderer& renderer) const;
private:
	std::unique_ptr<Technique> pTechnique;
};