#pragma once
#include <string>
#include "MeshRenderer.h"
#include "SceneGraphNode.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "FrameCommander.h"

namespace Rgph
{
	class FrameCommander;
}

namespace Bind
{
	class Material;
}

class Node
{
	friend class ModelInstance;
public:
	Node(int id, const DirectX::XMMATRIX& _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<Node>> pChildNodes);
	void SubmitDrawCalls(Rgph::FrameCommander& frame, DirectX::FXMMATRIX accumulatedTransform) const;
	void SetAppliedTransform(DirectX::FXMMATRIX transform);
	const DirectX::XMFLOAT4X4& GetAppliedTransform() const;
private:
	int id;
	std::vector<std::unique_ptr<Node>> pChildNodes;
	std::unique_ptr<MeshRenderer> pMeshPtr;
	DirectX::XMFLOAT4X4 transform;
	DirectX::XMFLOAT4X4 appliedTransform;
};

class ModelInstance
{
public:
	ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, dx::XMMATRIX transform);
	//ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, std::vector<std::string> materialPaths, dx::XMMATRIX transform);
	void SubmitDrawCalls(Rgph::FrameCommander& frame) const;
	void SetPositionWS(DirectX::XMFLOAT3 positionWS);
private:
	std::unique_ptr<MeshRenderer> ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset);
	std::unique_ptr<Node> CreateModelInstanceNode(Graphics& gfx, std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode);
private:
	std::vector<std::shared_ptr<Bind::Material>> pMaterials;
	std::unique_ptr<Node> pSceneGraph;
	std::vector<std::unique_ptr<MeshRenderer>> pMeshes;
	DirectX::XMMATRIX transform;
};
