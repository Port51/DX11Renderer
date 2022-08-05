#pragma once
#include "GameObject.h"
#include "CommonHeader.h"

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

	class ModelInstance : public GameObject
	{
	public:
		ModelInstance(const GraphicsDevice& gfx, const ModelAsset& pModelAsset, const dx::XMMATRIX& transform);
		ModelInstance(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform);
		virtual ~ModelInstance();
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
		std::shared_ptr<SceneGraphNode> CreateModelInstanceNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode);
	private:
		std::vector<std::shared_ptr<Material>> m_pMaterials;
		std::shared_ptr<SceneGraphNode> m_pSceneGraph;
		std::vector<std::shared_ptr<MeshRenderer>> m_pMeshes;
	};
}