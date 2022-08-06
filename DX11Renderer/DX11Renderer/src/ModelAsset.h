#pragma once
#include "CodexElement.h"
#include "SceneGraphNode.h"

namespace gfx
{
	class MeshAsset;
	class Model;
	class ModelImporter;

	class ModelAssetNode
	{
		friend class Model;
		friend class ModelImporter;

	public:
		ModelAssetNode(std::string name, std::shared_ptr<MeshAsset> pMeshAsset, dx::XMFLOAT4X4 localTransform);
		void Release();
	public:
		MeshAsset* GetMeshAsset() const;
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
		friend class Model;
		friend class ModelImporter;

	public:
		ModelAsset(std::shared_ptr<ModelAssetNode> _pSceneGraph, std::vector<std::string> _materialPaths);
		virtual ~ModelAsset() = default;
		virtual void Release() override;

	public:
		std::shared_ptr<Model> CreateSharedInstance(const GraphicsDevice& gfx, dx::XMMATRIX transform);
		std::unique_ptr<Model> CreateUniqueInstance(const GraphicsDevice& gfx, dx::XMMATRIX transform);

	private:
		std::shared_ptr<ModelAssetNode> m_pSceneGraph;
		std::vector<std::string> m_materialPaths;
	};
}