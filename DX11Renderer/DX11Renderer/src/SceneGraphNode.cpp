#include "pch.h"
#include "SceneGraphNode.h"
#include "MeshRenderer.h"
#include "DrawContext.h"

namespace gfx
{
	SceneGraphNode::SceneGraphNode(int id, const dx::XMMATRIX & _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes)
		: m_pMeshPtr(pMeshPtr), m_pChildNodes(std::move(pChildNodes))
	{
		dx::XMStoreFloat4x4(&m_localTransform, _transform);
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