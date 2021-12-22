#pragma once
#include <vector>
#include <memory>

class Renderer;
class Frustum;
class MeshRenderer;
class ModelInstance;
struct DrawContext;

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
	void SubmitDrawCalls(Renderer& renderer, const DrawContext& drawContext) const;
	void AddModelInstance(const ModelInstance& modelInstance);
	void AddMeshRenderer(const MeshRenderer& meshRenderer);
private:
	static bool SortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
	static bool SortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
private:
	// Float measures view distance
	std::shared_ptr<RendererList> pSource;
	std::vector<std::pair<std::shared_ptr<MeshRenderer>, float>> pRenderers;
};