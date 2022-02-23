#pragma once
#include "BaseSceneGraphNode.h"
#include "CommonHeader.h"
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <type_traits>
#include "DXMathInclude.h"
#include "BaseMesh.h"

namespace gfx
{
	template<class M>
	class SceneGraphNode2 : public BaseSceneGraphNode
	{
		friend class ModelInstance;
	public:
		SceneGraphNode() {}
		SceneGraphNode(dx::XMMATRIX localTransform, std::shared_ptr<M> _pMesh)
			: localTransform(std::move(localTransform)),
			pMesh(std::move(_pMesh))
		{
			// C++11 <type_traits> ensures M is a valid mesh type
			static_assert(std::is_base_of<BaseMesh, M>::value, "M must derive from BaseMesh");
		}
		virtual ~SceneGraphNode() = default;
	public:
		void SetChildNodes(std::vector<std::shared_ptr<SceneGraphNode<M>>> _pChildNodes)
		{
			pChildNodes = std::move(_pChildNodes);
		}
	private:
		std::shared_ptr<M> pMesh;
		std::vector<std::shared_ptr<SceneGraphNode<M>>> pChildNodes;
	};
}