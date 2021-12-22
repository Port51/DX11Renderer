#include "RendererList.h"
#include "Frustum.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"

RendererList::RendererList(std::shared_ptr<RendererList> source)
	: pSource(source)
{
	pRenderers.reserve(pSource->pRenderers.size());
}

bool RendererList::sortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b)
{
	return (a.second < b.second);
}

bool RendererList::sortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b)
{
	return (a.second > b.second);
}

void RendererList::Filter(Frustum frustum, RendererSorting sorting)
{
	// Filter based on source
	pRenderers.clear();
	for (const auto& p : pSource->pRenderers)
	{
		// todo: frustum cull AABB
		// todo: get distance
		pRenderers.emplace_back(std::make_pair(p.first, 0.f));
	}

	switch (sorting)
	{
	case RendererSorting::BackToFront:
		std::sort(pRenderers.begin(), pRenderers.end(), sortBackToFront);
		break;
	case RendererSorting::FrontToBack:
		std::sort(pRenderers.begin(), pRenderers.end(), sortFrontToBack);
		break;
	default:
		throw std::runtime_error("Unrecognized sorting method!");
		break;
	}
	
}

void RendererList::AddModelInstance(const ModelInstance & modelInstance)
{
	const auto pModelRenderers = modelInstance.GetMeshRenderers();
	for (const auto pr : pModelRenderers)
	{
		AddMeshRenderer(*pr);
	}
}

void RendererList::AddMeshRenderer(const MeshRenderer & meshRenderer)
{
	pRenderers.emplace_back(std::make_pair(std::make_unique<MeshRenderer>(meshRenderer), 0.f));
}
