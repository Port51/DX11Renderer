#pragma once
#include <vector>
#include <memory>

class Frustum;
class MeshRenderer;
class ModelInstance;

class RendererList
{
public:
	enum RendererSorting
	{
		BackToFront,
		FrontToBack
	};
public:
	RendererList() {}
	RendererList(const RendererList& source);
public:
	void Filter(Frustum frustum, RendererSorting sorting);
	void AddModelInstance(const ModelInstance& modelInstance);
	void AddMeshRenderer(const MeshRenderer& meshRenderer);
private:
	// Float measures view distance
	std::vector<std::pair<std::shared_ptr<MeshRenderer>, float>> pRenderers;
};