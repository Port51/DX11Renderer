#include "RendererList.h"
#include "Frustum.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"

RendererList::RendererList(std::shared_ptr<RendererList> source)
	: pSource(source)
{
	pRenderers.reserve(pSource->pRenderers.size());
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

	// todo: sort
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
