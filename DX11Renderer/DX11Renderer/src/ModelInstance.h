#pragma once
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"
#include "SceneGraphNode.h"
#include "ModelNode.h"
#include "MeshRenderer.h"

namespace DirectX
{
	struct XMMATRIX;
	struct XMFLOAT3;
}

namespace gfx
{
	class Graphics;
	class Material;
	class MeshRenderer;
	class ModelNode;
	class ModelAsset;
	class MeshAsset;
	class Renderer;
	struct DrawContext;

	class ModelInstance
	{
	public:
		ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, dx::XMMATRIX transform);
		virtual ~ModelInstance() = default;
	public:
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void SetPositionWS(dx::XMFLOAT3 positionWS);
		void UpdateSceneGraph();
		std::vector<std::shared_ptr<MeshRenderer>> GetMeshRenderers() const;
	private:
		std::shared_ptr<MeshRenderer> ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset);
		std::unique_ptr<ModelNode> CreateModelInstanceNode(Graphics& gfx, std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode);
	private:
		std::vector<std::shared_ptr<Material>> pMaterials;
		std::unique_ptr<ModelNode> pSceneGraph;
		std::vector<std::shared_ptr<MeshRenderer>> pMeshes;
		dx::XMMATRIX transform;
	};
}