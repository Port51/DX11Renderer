#pragma once
#include <assert.h>
#include <stdio.h>
#include "SceneGraph.h"
#include "MeshAsset.h"
#include "DebugHelper.h"

class ModelAsset
{
	friend class ModelInstanceUtil;
public:
	ModelAsset(std::unique_ptr<SceneGraph<MeshAsset>> _pSceneGraph)
		: pSceneGraph(std::move(_pSceneGraph))
	{

	}

	void Print()
	{
		if (pSceneGraph)
		{
			printf("Model:\n");
			pSceneGraph->Print();
		}
		else
		{
			printf("No scene graph!\n");
		}
	}

private:
	std::unique_ptr<SceneGraph<MeshAsset>> pSceneGraph;
};