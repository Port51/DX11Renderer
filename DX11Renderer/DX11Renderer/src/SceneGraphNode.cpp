#include "pch.h"
#include "SceneGraphNode.h"
#include "MeshRenderer.h"
#include "DrawContext.h"
#include <queue>

namespace gfx
{
	SceneGraphNode::SceneGraphNode(const dx::XMMATRIX& _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes)
		: m_pMeshRenderer(pMeshPtr), m_pChildNodes(std::move(pChildNodes))
	{
		dx::XMStoreFloat4x4(&m_localTransform, _transform);

		// Decompose translation part of matrix
		dx::XMStoreFloat3(&m_localTransformOffset, DecomposeMatrixTranslation(m_localTransform));
	}

	SceneGraphNode::~SceneGraphNode()
	{}

	void SceneGraphNode::RebuildTransform(const dx::XMMATRIX& accumulatedTransform)
	{
		const auto worldMatrix = dx::XMLoadFloat4x4(&m_localTransform) * accumulatedTransform;
		dx::XMStoreFloat4x4(&m_accumulatedWorldTransform, worldMatrix);

		dx::XMStoreFloat3(&m_accumulatedWorldTransformOffset, DecomposeMatrixTranslation(m_accumulatedWorldTransform));

		if (m_pMeshRenderer)
		{
			m_pMeshRenderer->SetTransform(worldMatrix);
		}

		for (const auto& pc : m_pChildNodes)
		{
			pc->RebuildTransform(worldMatrix);
		}
	}

	void SceneGraphNode::RebuildBoundingVolumeHierarchy()
	{
		// Rebuilds all nodes in graph, in order of depth (deepest first)
		std::unordered_map<int, std::vector<std::pair<int, SceneGraphNode*>>> pNodesByDepth;

		// Create map using breadth-first traversal
		std::queue<std::pair<int, SceneGraphNode*>> queue;
		queue.emplace(std::make_pair(0u, this));

		int maxDepth = 0;

		while (!queue.empty())
		{
			auto first = queue.front();
			queue.pop();

			auto node = first.second;
			auto nodeDepth = first.first;
			auto nodePair = std::make_pair(nodeDepth, node);
			maxDepth = std::max(maxDepth, nodeDepth);

			// Add to map
			if (pNodesByDepth.find(nodeDepth) == pNodesByDepth.end())
			{
				std::vector<std::pair<int, SceneGraphNode*>> depthVector;
				depthVector.emplace_back(nodePair);
				pNodesByDepth.emplace(std::make_pair(nodeDepth, std::move(depthVector)));
			}
			else
			{
				pNodesByDepth.at(nodeDepth).emplace_back(nodePair);
			}

			// Add children to queue
			for (size_t i = 0, ct = node->m_pChildNodes.size(); i < ct; ++i)
			{
				queue.emplace(std::make_pair(nodeDepth + 1, node->m_pChildNodes.at(i).get()));
			}
		}

		// Rebuild deeper nodes first
		// No need to rebuild parents, as they will be built after all their children have been
		for (int depth = maxDepth; depth >= 0; --depth)
		{
			auto pNodes = pNodesByDepth.at(depth);
			for (size_t i = 0, ct = pNodes.size(); i < ct; ++i)
			{
				pNodes[i].second->RebuildBoundingVolume(false);
			}
		}
	}

	void SceneGraphNode::RebuildBoundingVolume(bool rebuildParents)
	{
		m_boundingVolumeAABB.Clear();
		if (m_pMeshRenderer != nullptr)
		{
			m_boundingVolumeAABB.ExpandBoundsToFitAABB(m_pMeshRenderer->GetAABB());
		}
		m_boundingVolumeAABB.ExpandBoundsToFitChildNodes(m_pChildNodes);

		if (rebuildParents && m_pParentNode != nullptr)
		{
			m_pParentNode->RebuildBoundingVolume(true);
		}
	}

	const std::vector<std::shared_ptr<SceneGraphNode>>& SceneGraphNode::GetChildren() const
	{
		return m_pChildNodes;
	}

	const std::shared_ptr<MeshRenderer>& SceneGraphNode::GetMeshRenderer() const
	{
		return m_pMeshRenderer;
	}

	const AABB& SceneGraphNode::GetBoundingVolume() const
	{
		return m_boundingVolumeAABB;
	}

	const dx::XMVECTOR SceneGraphNode::GetPositionWS() const
	{
		return dx::XMLoadFloat3(&m_accumulatedWorldTransformOffset);
	}

	/*void SceneGraphNode::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		if (m_pMeshRenderer)
		{
			m_pMeshRenderer->SubmitDrawCommands(drawContext);
		}

		for (const auto& pc : m_pChildNodes)
		{
			pc->SubmitDrawCalls(drawContext);
		}
	}*/
}