#include "pch.h"
#include "SceneGraphNode.h"
#include "MeshRenderer.h"
#include "DrawContext.h"

namespace gfx
{
	SceneGraphNode::SceneGraphNode(int id, const dx::XMMATRIX & _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes)
		: pMeshPtr(pMeshPtr), pChildNodes(std::move(pChildNodes))
	{
		dx::XMStoreFloat4x4(&localTransform, _transform);
	}

	void SceneGraphNode::RebuildTransform(dx::XMMATRIX accumulatedTransform)
	{
		const auto worldMatrix = dx::XMLoadFloat4x4(&localTransform) * accumulatedTransform;
		dx::XMStoreFloat4x4(&accumulatedWorldTransform, worldMatrix);

		if (pMeshPtr)
		{
			pMeshPtr->SetTransform(worldMatrix);
		}

		for (const auto& pc : pChildNodes)
		{
			pc->RebuildTransform(worldMatrix);
		}
	}

	void SceneGraphNode::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		if (pMeshPtr)
		{
			pMeshPtr->SubmitDrawCalls(drawContext);
		}

		for (const auto& pc : pChildNodes)
		{
			pc->SubmitDrawCalls(drawContext);
		}
	}
}