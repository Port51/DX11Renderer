#pragma once
#include "SceneGraphNode.h"
#include "MeshAsset.h"

namespace gfx
{
	class ModelAsset
	{
		friend class ModelInstance;
	public:
		ModelAsset(std::unique_ptr<SceneGraphNode<MeshAsset>> _pSceneGraph, std::vector<std::string> _materialPaths);
		virtual ~ModelAsset() = default;

		void Print();

	private:
		std::unique_ptr<SceneGraphNode<MeshAsset>> pSceneGraph;
		std::vector<std::string> materialPaths;
	};
}