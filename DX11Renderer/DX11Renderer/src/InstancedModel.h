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
	class VertexBufferWrapper;
	struct DrawContext;

	template<typename Type>
	class StructuredBufferData;

	struct InstanceData
	{
		dx::XMFLOAT4X4 transform;
		dx::XMFLOAT4 color;
	};

	class InstancedModel : public BaseModel
	{
	public:
		InstancedModel(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform, const std::vector<TRS>& instancePositions);
		virtual ~InstancedModel();

	public:
		const size_t GetInstanceCount() const;
		StructuredBufferData<InstanceData>& GetInstanceData() const;
		InstanceData GetInstanceDataPoint(const size_t i) const;
		void SetInstanceDataPoint(const size_t i, const InstanceData& data);
		void ApplyInstanceData(const GraphicsDevice& gfx);
		//void SubmitDrawCalls(const DrawContext& drawContext) const;
		//const std::vector<std::shared_ptr<InstancedMeshRenderer>>& GetMeshRenderers() const;

	protected:
		std::shared_ptr<InstancedMeshRenderer> CreateInstancedMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset, const std::vector<TRS>& instancePositions);
		std::shared_ptr<SceneGraphNode> CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode, const std::vector<TRS>& instancePositions);

	protected:
		size_t m_instanceCount;
		std::vector<std::shared_ptr<InstancedMeshRenderer>> m_pMeshRenderers;
		std::unique_ptr<StructuredBufferData<InstanceData>> m_pInstanceBuf;
		std::vector<std::shared_ptr<VertexBufferWrapper>> m_pVertexBuffers;

	};
}