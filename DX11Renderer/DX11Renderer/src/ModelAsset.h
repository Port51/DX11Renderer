#pragma once
#include "CodexElement.h"
#include "SceneGraphNode.h"

namespace gfx
{
	class MeshAsset;
	class ModelInstance;
	class ModelImporter;

	class ModelAssetNode
	{
		friend class ModelInstance;
		friend class ModelImporter;

	public:
		ModelAssetNode(std::string name, std::shared_ptr<MeshAsset> pMeshAsset, dx::XMFLOAT4X4 localTransform);
	public:
		const std::shared_ptr<MeshAsset> GetMeshAsset() const;
		const dx::XMFLOAT4X4 GetLocalTransform() const;
		void SetChildNodes(std::vector<std::shared_ptr<ModelAssetNode>> pChildNodes);
	private:
		std::string m_name;
		std::shared_ptr<MeshAsset> m_pMeshAsset;
		dx::XMFLOAT4X4 m_localTransform;
		std::vector<std::shared_ptr<ModelAssetNode>> m_pChildNodes;
	};

	class ModelAsset : CodexElement
	{
		friend class ModelInstance;
		friend class ModelImporter;

	public:
		ModelAsset(std::shared_ptr<ModelAssetNode> _pSceneGraph, std::vector<std::string> _materialPaths);
		virtual ~ModelAsset() = default;

	public:
		std::shared_ptr<ModelInstance> CreateSharedInstance(const GraphicsDevice& gfx, dx::XMMATRIX transform);
		std::unique_ptr<ModelInstance> CreateUniqueInstance(const GraphicsDevice& gfx, dx::XMMATRIX transform);

	private:
		std::shared_ptr<ModelAssetNode> m_pSceneGraph;
		std::vector<std::string> m_materialPaths;
	};
}