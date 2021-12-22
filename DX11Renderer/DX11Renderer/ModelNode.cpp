#include "ModelNode.h"
#include "MeshRenderer.h"

ModelNode::ModelNode(int id, const dx::XMMATRIX & _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<ModelNode>> pChildNodes)
	: pMeshPtr(pMeshPtr), pChildNodes(std::move(pChildNodes))
{
	dx::XMStoreFloat4x4(&localTransform, _transform);
}

void ModelNode::RebuildTransform(dx::XMMATRIX accumulatedTransform)
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

void ModelNode::SubmitDrawCalls(Renderer& renderer) const
{
	if (pMeshPtr)
	{
		pMeshPtr->SubmitDrawCalls(renderer);
	}

	for (const auto& pc : pChildNodes)
	{
		pc->SubmitDrawCalls(renderer);
	}
}
