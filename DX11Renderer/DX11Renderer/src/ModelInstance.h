#pragma once
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"
#include "SceneGraphNode.h"
#include "MeshRenderer.h"
//#include "ModelAsset.h"

namespace gfx
{
	class Graphics;
	class Material;
	class MeshRenderer;
	class SceneGraphNode;
	class ModelAsset;
	class ModelAssetNode;
	class MeshAsset;
	struct DrawContext;

	class ModelInstance
	{
	public:
		ModelInstance(Graphics& gfx, const ModelAsset& pModelAsset, dx::XMMATRIX transform);
		ModelInstance(Graphics& gfx, std::shared_ptr<ModelAsset> pModelAsset, dx::XMMATRIX transform);
		virtual ~ModelInstance() = default;
	public:
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void SetPositionWS(dx::XMFLOAT3 positionWS);
		void UpdateSceneGraph();
		std::vector<std::shared_ptr<MeshRenderer>> GetMeshRenderers() const;
	private:
		std::shared_ptr<MeshRenderer> CreateMeshRenderer(Graphics& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset);
		std::shared_ptr<SceneGraphNode> CreateModelInstanceNode(Graphics& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode);
	private:
		std::vector<std::shared_ptr<Material>> pMaterials;
		std::shared_ptr<SceneGraphNode> pSceneGraph;
		std::vector<std::shared_ptr<MeshRenderer>> pMeshes;
		dx::XMMATRIX transform;
	};
}