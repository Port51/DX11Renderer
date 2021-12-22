#include "RendererList.h"
#include "Frustum.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"

RendererList::RendererList(const RendererList & source)
{
	// todo: find a more efficient way of doing this

	// Copy renderers
	for (const auto& p : source.pRenderers)
	{
		pRenderers.emplace_back(std::make_pair(p.first, 0.f));
	}
	
}

void RendererList::Filter(Frustum frustum, RendererSorting sorting)
{
	// todo: frustum cull AABB
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
