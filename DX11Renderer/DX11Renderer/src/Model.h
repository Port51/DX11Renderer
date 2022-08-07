#pragma once
#include "BaseModel.h"
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

	class Model : public BaseModel
	{
	public:
		Model(const GraphicsDevice& gfx, const ModelAsset& pModelAsset, const dx::XMMATRIX& transform);
		Model(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform);
		virtual ~Model();
	public:
		//void SubmitDrawCalls(const DrawContext& drawContext) const;
		//const std::vector<std::shared_ptr<MeshRenderer>>& GetMeshRenderers() const;
	protected:
		std::shared_ptr<MeshRenderer> CreateMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset);
		std::shared_ptr<SceneGraphNode> CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode);
	protected:
		std::vector<std::shared_ptr<MeshRenderer>> m_pMeshRenderers;
	};
}