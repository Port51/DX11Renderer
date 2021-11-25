#pragma once
#include <string>
#include <d3d11.h>
#include <memory>
#include "SceneGraphNode.h"

class Graphics;
class Material;
class ModelNode;
class MeshRenderer;
class ModelAsset;
class MeshAsset;
class FrameCommander;

namespace DirectX
{
	struct XMMATRIX;
	struct XMFLOAT3;
}

class ModelInstance
{
public:
	ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, dx::XMMATRIX transform);
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame) const;
	void SetPositionWS(dx::XMFLOAT3 positionWS);
private:
	std::unique_ptr<MeshRenderer> ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset);
	std::unique_ptr<ModelNode> CreateModelInstanceNode(Graphics& gfx, std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode);
private:
	std::vector<std::shared_ptr<Material>> pMaterials;
	std::unique_ptr<ModelNode> pSceneGraph;
	std::vector<std::unique_ptr<MeshRenderer>> pMeshes;
	dx::XMMATRIX transform;
};
