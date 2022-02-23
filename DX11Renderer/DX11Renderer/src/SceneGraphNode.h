#pragma once
#include "ModelInstance.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"
#include "DXMathInclude.h"
#include "SceneGraphNode.h"

namespace gfx
{
	class MeshRenderer;
	class ModelAsset;
	class MeshAsset;
	class Renderer;
	struct DrawContext;

	class SceneGraphNode
	{
		friend class ModelInstance;
	public:
		SceneGraphNode(int id, const dx::XMMATRIX& _transform, std::shared_ptr<MeshRenderer> pMeshPtr, std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes);
		virtual ~SceneGraphNode() = default;
	public:
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void RebuildTransform(dx::XMMATRIX accumulatedTransform);
	private:
		int id;
		std::vector<std::shared_ptr<SceneGraphNode>> pChildNodes;
		std::shared_ptr<MeshRenderer> pMeshPtr;
		dx::XMFLOAT4X4 localTransform;
		dx::XMFLOAT4X4 accumulatedWorldTransform;
	};
}