#pragma once
#include "GameObject.h"
#include "CommonHeader.h"
#include <vector>

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

	class Model : public GameObject
	{
	public:
		Model(const GraphicsDevice& gfx, const ModelAsset& pModelAsset, const dx::XMMATRIX& transform);
		Model(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform);
		virtual ~Model();
	public:
		//void SubmitDrawCalls(const DrawContext& drawContext) const;
		void RebuildSceneGraphTransforms();
		const std::shared_ptr<SceneGraphNode> GetSceneGraph() const;
		const std::vector<std::shared_ptr<MeshRenderer>>& GetMeshRenderers() const;
	protected:
		virtual void ApplyTRS() override;
	private:
		void InitializeModel();
	private:
		std::shared_ptr<MeshRenderer> CreateMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset);
		std::shared_ptr<SceneGraphNode> CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode);
	private:
		std::vector<std::shared_ptr<Material>> m_pMaterials;
		std::shared_ptr<SceneGraphNode> m_pSceneGraph;
		std::vector<std::shared_ptr<MeshRenderer>> m_pMeshes;
	};
}