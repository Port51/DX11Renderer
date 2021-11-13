#pragma once
#include <assert.h>
#include <stdio.h>
#include "SceneGraphNode.h"
#include "MeshAsset.h"
#include "DebugHelper.h"

class ModelAsset
{
	friend class ModelInstance;
public:
	ModelAsset(std::unique_ptr<SceneGraphNode<MeshAsset>> _pSceneGraph, std::vector<std::string> _materialPaths)
		: pSceneGraph(std::move(_pSceneGraph)), materialPaths(std::move(_materialPaths))
	{

	}

	void Print()
	{
		if (pSceneGraph)
		{
			printf("Model:\n");
			pSceneGraph->Print(0);
		}
		else
		{
			printf("No scene graph!\n");
		}
	}

private:
	std::unique_ptr<SceneGraphNode<MeshAsset>> pSceneGraph;
	std::vector<std::string> materialPaths;
};