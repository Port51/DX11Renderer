#pragma once
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"
#include "SceneGraphNode.h"
#include "MeshRenderer.h"
//#include "ModelAsset.h"

namespace gfx
{
	class GraphicsDevice;
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
		ModelInstance(GraphicsDevice& gfx, const ModelAsset& pModelAsset, dx::XMMATRIX transform);
		ModelInstance(GraphicsDevice& gfx, std::shared_ptr<ModelAsset> pModelAsset, dx::XMMATRIX transform);
		virtual ~ModelInstance() = default;
	public:
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void SetPositionWS(dx::XMFLOAT3 positionWS);
		void UpdateSceneGraph();
		const std::vector<std::shared_ptr<MeshRenderer>> GetMeshRenderers() const;
	private:
		std::shared_ptr<MeshRenderer> CreateMeshRenderer(GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset);
		std::shared_ptr<SceneGraphNode> CreateModelInstanceNode(GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode);
	private:
		std::vector<std::shared_ptr<Material>> m_pMaterials;
		std::shared_ptr<SceneGraphNode> m_pSceneGraph;
		std::vector<std::shared_ptr<MeshRenderer>> m_pMeshes;
		dx::XMMATRIX m_transform;
	};
}