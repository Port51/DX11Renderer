#pragma once
#include "GameObject.h"
#include "CommonHeader.h"
#include "RawBufferData.h"
#include <vector>

namespace gfx
{
	class GraphicsDevice;
	class Material;
	class MeshRenderer;
	class SceneGraphNode;
	class ModelAsset;
	class ModelAssetNode;
	class MeshAsset;
	struct DrawContext;

	class BaseModel : public GameObject
	{
	public:
		const std::shared_ptr<SceneGraphNode> GetSceneGraph() const;

	protected:
		const RawBufferData CreateVertexBufferData(std::shared_ptr<MeshAsset> const& pMeshAsset, std::shared_ptr<Material> const& pMaterial) const;
		virtual void ApplyTRS() override;
		void RebuildSceneGraphTransforms();
		void InitializeModel();

	protected:
		std::vector<std::shared_ptr<Material>> m_pMaterials;
		std::shared_ptr<SceneGraphNode> m_pSceneGraph;
	};
}