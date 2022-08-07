#pragma once
#include "Model.h"
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
		friend class Model;
		friend class InstancedModel;
	public:
		SceneGraphNode(const dx::XMMATRIX& _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes);
		virtual ~SceneGraphNode() = default;
	public:
		//void SubmitDrawCalls(const DrawContext& drawContext) const;
		void RebuildTransform(dx::XMMATRIX accumulatedTransform);
		// Rebuild entire BVH, starting at this level
		void RebuildBoundingVolumeHierarchy();
		// Rebuild current BVH AABB based on children
		void RebuildBoundingVolume(bool rebuildParents);
		const std::vector<std::shared_ptr<SceneGraphNode>>& GetChildren() const;
		const std::shared_ptr<MeshRenderer>& GetMeshRenderer() const;
		const AABB& GetBoundingVolume() const;
		const dx::XMVECTOR GetPositionWS() const;
	private:
		int m_id;
		std::shared_ptr<SceneGraphNode> m_pParentNode;
		std::vector<std::shared_ptr<SceneGraphNode>> m_pChildNodes;
		std::shared_ptr<MeshRenderer> m_pMeshRenderer;
		// Local TRS transform
		dx::XMFLOAT4X4 m_localTransform;
		dx::XMFLOAT4X4 m_accumulatedWorldTransform;
		// Translation portions of TRS's
		dx::XMFLOAT3 m_localTransformOffset;
		dx::XMFLOAT3 m_accumulatedWorldTransformOffset;
		// AABB that encapsulates any object meshes at this node level or deeper
		AABB m_boundingVolumeAABB;
	};
}