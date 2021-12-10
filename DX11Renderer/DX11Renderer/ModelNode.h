#pragma once
#include "ModelInstance.h"
#include <string>
#include <memory>
#include <d3d11.h>
#include "DXMathInclude.h"
#include "SceneGraphNode.h"

class MeshRenderer;
class ModelAsset;
class MeshAsset;
class Renderer;

class ModelNode
{
	friend class ModelInstance;
public:
	ModelNode(int id, const dx::XMMATRIX& _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<ModelNode>> pChildNodes);
	void SubmitDrawCalls(std::unique_ptr<Renderer>& frame, dx::FXMMATRIX accumulatedTransform) const;
	void SetAppliedTransform(dx::FXMMATRIX transform);
	const dx::XMFLOAT4X4& GetAppliedTransform() const;
private:
	int id;
	std::vector<std::unique_ptr<ModelNode>> pChildNodes;
	std::unique_ptr<MeshRenderer> pMeshPtr;
	dx::XMFLOAT4X4 transform;
	dx::XMFLOAT4X4 appliedTransform;
};