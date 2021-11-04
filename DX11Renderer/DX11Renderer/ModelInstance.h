#pragma once
#include <string>
#include "Drawable.h"
#include "Mesh.h"
#include "SceneGraphNode.h"
#include "ModelAsset.h"
#include "MeshAsset.h"

class Node
{
	friend class ModelInstance;
public:
	Node(const DirectX::XMMATRIX& _transform, std::unique_ptr<Mesh> pMeshPtr, std::vector<std::unique_ptr<Node>> pChildNodes)
		: pMeshPtr(std::move(pMeshPtr)), pChildNodes(std::move(pChildNodes))
	{
		DirectX::XMStoreFloat4x4(&localTransform, _transform);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
private:
	std::vector<std::unique_ptr<Node>> pChildNodes;
	std::unique_ptr<Mesh> pMeshPtr;
	DirectX::XMFLOAT4X4 localTransform;
};

class ModelInstance
{
public:
	ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, DirectX::XMFLOAT3 materialColor, dx::XMMATRIX transform);
	void Draw(Graphics& gfx) const;
private:
	// todo: move these?
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset);
	static std::unique_ptr<Node> CreateModelInstanceNode(Graphics& gfx, std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode);
private:
	std::unique_ptr<Node> pSceneGraph;
	std::vector<std::unique_ptr<Mesh>> pMeshes;
	DirectX::XMMATRIX transform;
};
