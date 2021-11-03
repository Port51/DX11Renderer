#pragma once
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <type_traits>
#include "DebugHelper.h"
#include "BaseMesh.h"

template<class M>
class SceneGraphNode
{
	friend class ModelInstance;
public:
	SceneGraphNode(DirectX::XMMATRIX localTransform, std::unique_ptr<M> _pMesh, std::vector<std::unique_ptr<SceneGraphNode<M>>> _pChildNodes)
		: localTransform(localTransform),
		pMesh(std::move(_pMesh)),
		pChildNodes(std::move(_pChildNodes))
	{
		// C++11 <type_traits> ensures M is a valid mesh type
		static_assert(std::is_base_of<BaseMesh, M>::value, "M must derive from BaseMesh");

		// Transfer ownership of vector elements
		/*for (const auto& child : _pChildNodes)
		{
			pChildNodes.emplace_back(std::move(child));
		}*/
	}

	/*void AddChildNode(SceneGraphNode<M> pNode)
	{
		pChildNodes.emplace_back(std::move(pNode));
	}*/

	void Print(int level)
	{
		if (pMesh)
		{
			DebugHelper::PrintTabs(level);
			printf("<MESH NODE ");
			printf(pMesh->name.c_str());
			printf(">\n");
		}
		else
		{
			DebugHelper::PrintTabs(level);
			printf("<empty node>\n");
		}

		for (const auto& child : pChildNodes)
		{
			child->Print(level + 1);
		}
	}

private:
	std::unique_ptr<M> pMesh;
	std::vector<std::unique_ptr<SceneGraphNode<M>>> pChildNodes;
	DirectX::XMMATRIX localTransform;
};