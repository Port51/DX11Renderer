#include "pch.h"
#include "GraphicsDevice.h"
#include "RenderStats.h"
#include "RendererList.h"
#include "Frustum.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
#include "Renderer.h"
#include "SceneGraphNode.h"
#include <queue>

namespace gfx
{
	RendererList::RendererList(std::shared_ptr<RendererList> source)
		: m_pSource(source)
	{
		m_pRenderers.reserve(m_pSource->m_pRenderers.size());
	}

	bool RendererList::SortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b)
	{
		return (a.second < b.second);
	}

	bool RendererList::SortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b)
	{
		return (a.second > b.second);
	}

	const UINT RendererList::GetRendererCount() const
	{
		return (UINT)m_pRenderers.size();
	}

	void RendererList::Filter(const GraphicsDevice& gfx, const Frustum& frustum, RendererSorting sorting)
	{
		// Frustum culling
		{
			// Use queue to avoid recursion
			std::queue<std::shared_ptr<SceneGraphNode>> nodeQueue;
			m_pRenderers.clear();
			for (const auto& p : m_pSource->m_pSceneGraphs)
			{
				nodeQueue.emplace(p);
			}

			while (!nodeQueue.empty())
			{
				const auto& node = nodeQueue.front();
				nodeQueue.pop();

				// Cull against BVH
				dx::XMVECTOR nodePositionWS = node->GetPositionWS();
				bool bvhInFrustum = frustum.DoesAABBIntersect(node->GetBoundingVolume(), nodePositionWS);
				if (bvhInFrustum)
				{
					// Add children
					for (const auto& child : node->GetChildren())
					{
						nodeQueue.emplace(child);
					}

					// Add renderer, if exists and is in frustum
					const auto& renderer = node->GetMeshRenderer();
					if (renderer != nullptr
						&& frustum.DoesAABBIntersect(renderer->GetAABB(), nodePositionWS))
					{
						// todo: get distance
						m_pRenderers.emplace_back(std::make_pair(renderer, 0.f));
						gfx.GetRenderStats().AddVisibleRenderers(1u);
					}
				}
			}
		}
		
		// todo: make sort keys to minimize shader program changes
		switch (sorting)
		{
		case RendererSorting::BackToFront:
			std::sort(m_pRenderers.begin(), m_pRenderers.end(), SortBackToFront);
			break;
		case RendererSorting::FrontToBack:
			std::sort(m_pRenderers.begin(), m_pRenderers.end(), SortFrontToBack);
			break;
		default:
			throw std::runtime_error("Unrecognized sorting method!");
			break;
		}

	}

	void RendererList::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		for (const auto pr : m_pRenderers)
		{
			pr.first->SubmitDrawCalls(drawContext);
		}
	}

	void RendererList::AddModelInstance(const ModelInstance& modelInstance)
	{
		m_pSceneGraphs.emplace_back(std::move(modelInstance.GetSceneGraph()));
	}

	void RendererList::AddSceneGraph(const std::shared_ptr<SceneGraphNode> pSceneGraph)
	{
		m_pSceneGraphs.emplace_back(std::move(pSceneGraph));
	}
}