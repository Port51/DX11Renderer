#pragma once
#include <memory>

class Technique;
class MeshRenderer;
class Renderer;

///
/// Collection of bindables needed to execute a material pass in a technique
///
class MaterialPass
{
public:
	MaterialPass();
public:
	void AddTechnique(std::unique_ptr<Technique> _pTechnique);
	void SubmitDrawCalls(Renderer& renderer, const MeshRenderer& meshRenderer) const;
private:
	std::unique_ptr<Technique> pTechnique;
};