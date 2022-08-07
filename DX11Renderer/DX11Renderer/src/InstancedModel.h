#pragma once
#include "BaseModel.h"
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class GraphicsDevice;
	class Material;
	class InstancedMeshRenderer;
	class SceneGraphNode;
	class ModelAsset;
	class ModelAssetNode;
	class MeshAsset;
	struct DrawContext;

	class InstancedModel : public BaseModel
	{
	public:
		InstancedModel(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform, const std::vector<TRS>& instancePositions);
		virtual ~InstancedModel();
	public:
		//void SubmitDrawCalls(const DrawContext& drawContext) const;
		//const std::vector<std::shared_ptr<InstancedMeshRenderer>>& GetMeshRenderers() const;
	protected:
		std::shared_ptr<InstancedMeshRenderer> CreateInstancedMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset, const std::vector<TRS>& instancePositions);
		std::shared_ptr<SceneGraphNode> CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode, const std::vector<TRS>& instancePositions);
	protected:
		size_t m_instanceCount;
		std::vector<std::shared_ptr<InstancedMeshRenderer>> m_pMeshRenderers;
	};
}