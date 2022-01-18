#include "pch.h"
#include "ModelAsset.h"
#include <assert.h>
#include <stdio.h>
#include "DebugHelper.h"

namespace gfx
{
	ModelAsset::ModelAsset(std::unique_ptr<SceneGraphNode<MeshAsset>> _pSceneGraph, std::vector<std::string> _materialPaths)
		: pSceneGraph(std::move(_pSceneGraph)), materialPaths(std::move(_materialPaths))
	{

	}

	void ModelAsset::Print()
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
}