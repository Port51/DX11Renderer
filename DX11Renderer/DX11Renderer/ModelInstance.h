#pragma once
#include <string>
#include "TestObject.h"
#include "Mesh.h"
#include "SceneGraphNode.h"
#include "MeshAsset.h"

class Node
{
	friend class ModelInstance;
public:
	Node(std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
private:
	void AddChild(std::unique_ptr<Node> pChild);
private:
	std::vector<std::unique_ptr<Node>> childPtrs;
	std::vector<Mesh*> meshPtrs;
	DirectX::XMFLOAT4X4 transform;
};

class ModelInstance
{
public:
	ModelInstance(Graphics& gfx, const std::string filename, DirectX::XMFLOAT3 materialColor, DirectX::XMFLOAT3 scale);
	void Draw(Graphics& gfx, DirectX::XMMATRIX transform) const;
private:
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const MeshAsset& mesh);
	std::unique_ptr<Node> ParseNode(const SceneGraphNode<MeshAsset>& node);
private:
	std::unique_ptr<Node> pSceneGraph;
	std::vector<std::unique_ptr<Mesh>> pMeshes;
};
