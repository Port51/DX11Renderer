#include "pch.h"
#include "RendererList.h"
#include "Frustum.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
#include "Renderer.h"

RendererList::RendererList(std::shared_ptr<RendererList> source)
	: pSource(source)
{
	pRenderers.reserve(pSource->pRenderers.size());
}

bool RendererList::SortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b)
{
	return (a.second < b.second);
}

bool RendererList::SortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b)
{
	return (a.second > b.second);
}

UINT RendererList::GetRendererCount() const
{
	return (UINT)pRenderers.size();
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
		std::sort(pRenderers.begin(), pRenderers.end(), SortBackToFront);
		break;
	case RendererSorting::FrontToBack:
		std::sort(pRenderers.begin(), pRenderers.end(), SortFrontToBack);
		break;
	default:
		throw std::runtime_error("Unrecognized sorting method!");
		break;
	}
	
}

void RendererList::SubmitDrawCalls(const DrawContext& drawContext) const
{
	for (const auto pr : pRenderers)
	{
		pr.first->SubmitDrawCalls(drawContext);
	}
}

void RendererList::AddModelInstance(const ModelInstance & modelInstance)
{
	const auto pModelRenderers = modelInstance.GetMeshRenderers();
	for (const auto pr : pModelRenderers)
	{
		AddMeshRenderer(pr);
	}
}

void RendererList::AddMeshRenderer(const std::shared_ptr<MeshRenderer> pMeshRenderer)
{
	pRenderers.emplace_back(std::make_pair(pMeshRenderer, 0.f));
}
