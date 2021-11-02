#pragma once
#include <memory>
#include "MeshAsset.h"
#include "MeshInstance.h"
#include "ModelAsset.h"
#include "ModelInstance.h"

class ModelInstanceUtil
{
public:
	static std::unique_ptr<ModelInstance> CreateModelInstance(std::unique_ptr<ModelAsset> const& pSource)
	{
		auto pSceneGraph = CreateModelInstanceSceneGraph(pSource->pSceneGraph);
		auto pInstance = std::make_unique<ModelInstance>(std::move(pSceneGraph));
		return std::move(pInstance);
	}

	static std::unique_ptr<SceneGraph<MeshInstance>> CreateModelInstanceSceneGraph(std::unique_ptr<SceneGraph<MeshAsset>> const& pSourceGraph)
	{
		auto pRootNode = CreateModelInstanceNode(pSourceGraph->pRootNode);
		auto pSceneGraph = std::make_unique<SceneGraph<MeshInstance>>(std::move(pRootNode));
		return std::move(pSceneGraph);
	}

	static std::unique_ptr<SceneGraphNode<MeshInstance>> CreateModelInstanceNode(std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode)
	{
		// Copy mesh if needed
		auto pMeshInstance = std::unique_ptr<MeshInstance>();
		if (pSourceNode->pMesh)
		{
			pMeshInstance = std::make_unique<MeshInstance>();
		}

		auto pChildNodes = std::vector<std::unique_ptr<SceneGraphNode<MeshInstance>>>();
		for (size_t i = 0; i < pSourceNode->pChildNodes.size(); ++i)
		{
			auto pChildNode = CreateModelInstanceNode(pSourceNode->pChildNodes[i]);
			pChildNodes.emplace_back(std::move(pChildNode));
		}

		auto pNode = std::make_unique<SceneGraphNode<MeshInstance>>(std::move(pMeshInstance), std::move(pChildNodes));
		return std::move(pNode);
	}

};