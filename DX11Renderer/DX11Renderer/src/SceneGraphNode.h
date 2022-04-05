#pragma once
#include "ModelInstance.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"
#include "DXMathInclude.h"
#include "AABB.h"

namespace gfx
{
	class MeshRenderer;
	class ModelAsset;
	class MeshAsset;
	class Renderer;
	struct DrawContext;

	class SceneGraphNode
	{
		friend class AABB;
		friend class ModelInstance;
	public:
		SceneGraphNode(int id, const dx::XMMATRIX& _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes);
		virtual ~SceneGraphNode() = default;
	public:
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void RebuildTransform(dx::XMMATRIX accumulatedTransform);
		void RebuildAABBHierarchy();
		void RebuildAABB(bool rebuildParents);
	private:
		int m_id;
		std::shared_ptr<SceneGraphNode> m_pParentNode;
		std::vector<std::shared_ptr<SceneGraphNode>> m_pChildNodes;
		std::shared_ptr<MeshRenderer> m_pMeshPtr;
		// Translation portion of TRS
		dx::XMFLOAT3 m_localTransformOffset;
		// Local TRS transform
		dx::XMFLOAT4X4 m_localTransform;
		dx::XMFLOAT4X4 m_accumulatedWorldTransform;
		AABB m_aabb;
	};
}