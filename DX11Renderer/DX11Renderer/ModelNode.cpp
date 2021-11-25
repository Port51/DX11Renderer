#include "ModelNode.h"
#include "MeshRenderer.h"

ModelNode::ModelNode(int id, const dx::XMMATRIX & _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<ModelNode>> pChildNodes)
	: pMeshPtr(std::move(pMeshPtr)), pChildNodes(std::move(pChildNodes))
{
	dx::XMStoreFloat4x4(&transform, _transform);
	dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}

void ModelNode::SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, dx::FXMMATRIX accumulatedTransform) const
{
	// todo: use this?
	const auto built =
		dx::XMLoadFloat4x4(&appliedTransform) *
		dx::XMLoadFloat4x4(&transform) *
		accumulatedTransform;

	if (pMeshPtr)
	{
		pMeshPtr->SubmitDrawCalls(frame, built);
	}

	for (const auto& pc : pChildNodes)
	{
		pc->SubmitDrawCalls(frame, built);
	}
}

void ModelNode::SetAppliedTransform(dx::FXMMATRIX _transform)
{
	dx::XMStoreFloat4x4(&appliedTransform, _transform);
}

const dx::XMFLOAT4X4& ModelNode::GetAppliedTransform() const
{
	return appliedTransform;
}
