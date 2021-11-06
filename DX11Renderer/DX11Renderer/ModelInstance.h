#pragma once
#include <string>
#include "Drawable.h"
#include "MeshRenderer.h"
#include "SceneGraphNode.h"
#include "ModelAsset.h"
#include "MeshAsset.h"

class Node
{
	friend class ModelInstance;
public:
	Node(const DirectX::XMMATRIX& _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<Node>> pChildNodes)
		: pMeshPtr(std::move(pMeshPtr)), pChildNodes(std::move(pChildNodes))
	{
		DirectX::XMStoreFloat4x4(&localTransform, _transform);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
private:
	std::vector<std::unique_ptr<Node>> pChildNodes;
	std::unique_ptr<MeshRenderer> pMeshPtr;
	DirectX::XMFLOAT4X4 localTransform;
};

class ModelInstance
{
public:
	ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, std::string materialPath, dx::XMMATRIX transform);
	ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, std::vector<std::string> materialPaths, dx::XMMATRIX transform);
	void Draw(Graphics& gfx) const;
	void SetPositionWS(DirectX::XMFLOAT3 positionWS);
private:
	std::unique_ptr<MeshRenderer> ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset);
	std::unique_ptr<Node> CreateModelInstanceNode(Graphics& gfx, std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode);
private:
	std::vector<std::shared_ptr<Material>> pMaterials;
	std::unique_ptr<Node> pSceneGraph;
	std::vector<std::unique_ptr<MeshRenderer>> pMeshes;
	DirectX::XMMATRIX transform;
};
