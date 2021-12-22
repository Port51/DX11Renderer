#include "ModelNode.h"
#include "MeshRenderer.h"

ModelNode::ModelNode(int id, const dx::XMMATRIX & _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<ModelNode>> pChildNodes)
	: pMeshPtr(std::move(pMeshPtr)), pChildNodes(std::move(pChildNodes))
{
	dx::XMStoreFloat4x4(&localTransform, _transform);
}

void ModelNode::RebuildTransform(dx::FXMMATRIX accumulatedTransform)
{
	const auto worldMatrix = dx::XMLoadFloat4x4(&localTransform) * accumulatedTransform;
	dx::XMStoreFloat4x4(&accumulatedWorldTransform, worldMatrix);

	for (const auto& pc : pChildNodes)
	{
		pc->RebuildTransform(worldMatrix);
	}
}

void ModelNode::SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const
{
	if (pMeshPtr)
	{
		pMeshPtr->SubmitDrawCalls(frame, dx::XMLoadFloat4x4(&accumulatedWorldTransform));
	}

	for (const auto& pc : pChildNodes)
	{
		pc->SubmitDrawCalls(frame);
	}
}
