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

	bool RendererList::SortByCode(const std::pair<std::shared_ptr<MeshRenderer>, u64>& a, const std::pair<std::shared_ptr<MeshRenderer>, u64>& b)
	{
		return (a.second < b.second);
	}

	const UINT RendererList::GetRendererCount() const
	{
		return (UINT)m_pRenderers.size();
	}

	void RendererList::Filter(const GraphicsDevice& gfx, const Frustum& frustum, const RendererSortingType sorting, const RenderPassType renderPassType, const dx::XMVECTOR originWS, const dx::XMVECTOR directionWS, const float farClipPlane)
	{
		const bool testDepth = (sorting != RendererSortingType::State);
		const bool reverseDepth = (sorting == RendererSortingType::BackToFrontThenState || sorting == RendererSortingType::StateThenBackToFront);
		const bool sortStateFirst = (sorting == RendererSortingType::StateThenBackToFront || sorting == RendererSortingType::StateThenFrontToBack);
		const float rcpFarClipPlane = 1.f / farClipPlane;

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
						// Then construct sorting code

						u64 code;
						if (testDepth)
						{
							// Combine depth and state

							// Get view depth as that doesn't require an inverse square root (slow)
							float depth = dx::XMVectorGetX(dx::XMVector3Dot(dx::XMVectorSubtract(renderer->GetAABB().GetCenterWS(nodePositionWS), originWS), directionWS));
							float depthPercentage = std::min(depth * rcpFarClipPlane, 1.f);
							if (reverseDepth)
							{
								depthPercentage = 1.f - depthPercentage;
							}

							const u64 depthCode = (u64)(4096u * depthPercentage); // use 12 bits for depth
							const u64 materialCode = renderer->GetMaterialCode(renderPassType);

							code = (sortStateFirst) ?
								(materialCode << 12u) + depthCode
								: (depthCode << 52u) + materialCode;
						}
						else
						{
							// Only sort by state
							code = renderer->GetMaterialCode(renderPassType);
						}
						

						m_pRenderers.emplace_back(std::make_pair(renderer, code));
						gfx.GetRenderStats().AddVisibleRenderers(1u);
					}
				}
			}
		}
		
		std::sort(m_pRenderers.begin(), m_pRenderers.end(), SortByCode);
	}

	void RendererList::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		for (const auto pr : m_pRenderers)
		{
			pr.first->SubmitDrawCommands(drawContext);
		}
	}

	void RendererList::AddModelInstance(const ModelInstance & modelInstance)
	{
		m_pSceneGraphs.emplace_back(modelInstance.GetSceneGraph());
	}

	void RendererList::AddSceneGraph(std::shared_ptr<SceneGraphNode> pSceneGraph)
	{
		m_pSceneGraphs.emplace_back(std::move(pSceneGraph));
	}
}