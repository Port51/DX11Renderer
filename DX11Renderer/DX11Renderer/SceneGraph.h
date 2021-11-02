#pragma once
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <type_traits>
#include "SceneGraphNode.h"
#include "DebugHelper.h"
#include "BaseMesh.h"

// Template on mesh type
template<class M>
class SceneGraph
{
	friend class ModelInstanceUtil;
public:
	template<class M>
	SceneGraph(std::unique_ptr<SceneGraphNode<M>> _pRootNode)
		: pRootNode(std::move(_pRootNode))
	{
		// C++11 <type_traits> ensures M is a valid mesh type
		static_assert(std::is_base_of<BaseMesh, M>::value, "M must derive from BaseMesh");
	}

	void Print()
	{
		if (pRootNode)
		{
			printf("Scene graph:\n");
			pRootNode->Print(0);
		}
		else
		{
			printf("No root node!\n");
		}
	}

private:
	std::unique_ptr<SceneGraphNode<M>> pRootNode;

};