#include "pch.h"
#include "SceneGraphNode.h"
#include "MeshRenderer.h"
#include "DrawContext.h"
#include <queue>

namespace gfx
{
	SceneGraphNode::SceneGraphNode(int id, const dx::XMMATRIX & _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes)
		: m_pMeshPtr(pMeshPtr), m_pChildNodes(std::move(pChildNodes))
	{
		dx::XMStoreFloat4x4(&m_localTransform, _transform);

		// Decompose translation part of matrix
		m_localTransformOffset = dx::XMFLOAT3(m_localTransform._14, m_localTransform._24, m_localTransform._34);
	}

	void SceneGraphNode::RebuildTransform(dx::XMMATRIX accumulatedTransform)
	{
		const auto worldMatrix = dx::XMLoadFloat4x4(&m_localTransform) * accumulatedTransform;
		dx::XMStoreFloat4x4(&m_accumulatedWorldTransform, worldMatrix);

		if (m_pMeshPtr)
		{
			m_pMeshPtr->SetTransform(worldMatrix);
		}

		for (const auto& pc : m_pChildNodes)
		{
			pc->RebuildTransform(worldMatrix);
		}
	}

	void SceneGraphNode::RebuildBoundingVolumeHierarchy()
	{
		// Rebuilds all nodes in graph, in order of depth (deepest first)
		std::unordered_map<int, std::vector<std::shared_ptr<SceneGraphNode>>> pNodesByDepth;

		// Create map using breadth-first traversal
		std::queue<std::pair<int, std::shared_ptr<SceneGraphNode>>> queue;
		queue.emplace(std::make_pair(0u, this));

		int maxDepth = 0;

		while (!queue.empty())
		{
			auto first = queue.front();
			queue.pop();

			auto node = first.second;
			auto nodeDepth = first.first;
			maxDepth = std::max(maxDepth, nodeDepth);

			// Add to map
			if (pNodesByDepth.find(nodeDepth) == pNodesByDepth.end())
			{
				std::vector<std::shared_ptr<SceneGraphNode>> depthVector;
				depthVector.emplace_back(node);
				pNodesByDepth.emplace(std::make_pair(nodeDepth, std::move(depthVector)));
			}
			else
			{
				pNodesByDepth.at(nodeDepth).emplace_back(node);
			}

			// Add children to queue
			for (int i = 0, ct = node->m_pChildNodes.size(); i < ct; ++i)
			{
				queue.emplace(std::make_pair(nodeDepth + 1, node->m_pChildNodes.at(i)));
			}
		}

		// Rebuild deeper nodes first
		// No need to rebuild parents, as they will be built after all their children have been
		for (int depth = maxDepth; depth >= 0; --depth)
		{
			auto pNodes = pNodesByDepth.at(depth);
			for (int i = 0, ct = pNodes.size(); i < ct; ++i)
			{
				pNodes[i]->RebuildBoundingVolume(false);
			}
		}
	}

	void SceneGraphNode::RebuildBoundingVolume(bool rebuildParents)
	{
		// todo: implement!
		m_boundingVolumeHierarchyAABB.Clear();
		if (m_pMeshPtr != nullptr)
		{
			m_boundingVolumeHierarchyAABB.ExpandBoundsToFitAABB(m_pMeshPtr->GetAABB());
		}
		m_boundingVolumeHierarchyAABB.ExpandBoundsToFitChildNodes(m_pChildNodes);

		if (rebuildParents && m_pParentNode != nullptr)
		{
			m_pParentNode->RebuildBoundingVolume(true);
		}
	}

	void SceneGraphNode::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		if (m_pMeshPtr)
		{
			m_pMeshPtr->SubmitDrawCalls(drawContext);
		}

		for (const auto& pc : m_pChildNodes)
		{
			pc->SubmitDrawCalls(drawContext);
		}
	}
}