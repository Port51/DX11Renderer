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
	RendererList(std::shared_ptr<RendererList> source);
public:
	void Filter(Frustum frustum, RendererSorting sorting);
	void AddModelInstance(const ModelInstance& modelInstance);
	void AddMeshRenderer(const MeshRenderer& meshRenderer);
private:
	static bool sortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
	static bool sortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
private:
	// Float measures view distance
	std::shared_ptr<RendererList> pSource;
	std::vector<std::pair<std::shared_ptr<MeshRenderer>, float>> pRenderers;
};