#pragma once
#include "ModelInstance.h"
#include <string>
#include <memory>
#include <d3d11.h>
#include "DX11MathInclude.h"
#include "SceneGraphNode.h"

class ModelNode;
class MeshRenderer;
class ModelAsset;
class MeshAsset;
class FrameCommander;

class ModelNode
{
	friend class ModelInstance;
public:
	ModelNode(int id, const DirectX::XMMATRIX& _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<ModelNode>> pChildNodes);
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, DirectX::FXMMATRIX accumulatedTransform) const;
	void SetAppliedTransform(DirectX::FXMMATRIX transform);
	const DirectX::XMFLOAT4X4& GetAppliedTransform() const;
private:
	int id;
	std::vector<std::unique_ptr<ModelNode>> pChildNodes;
	std::unique_ptr<MeshRenderer> pMeshPtr;
	DirectX::XMFLOAT4X4 transform;
	DirectX::XMFLOAT4X4 appliedTransform;
};