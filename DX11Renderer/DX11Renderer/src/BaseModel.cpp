#include "pch.h"
#include "BaseModel.h"
#include "MeshAsset.h"
#include "Material.h"
#include "SceneGraphNode.h"

namespace gfx
{
	const RawBufferData BaseModel::CreateVertexBufferData(std::shared_ptr<MeshAsset> const& pMeshAsset, std::shared_ptr<Material> const& pMaterial) const
	{
		RawBufferData vbuf(pMeshAsset->m_vertices.size(), pMaterial->GetVertexLayout().GetPerVertexStride(), pMaterial->GetVertexLayout().GetPerVertexPadding());

		if (pMeshAsset->m_vertices.size() == 0u)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has 0 vertices!"));
		}
		if (pMeshAsset->m_indices.size() == 0u)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has 0 indices!"));
		}
		if (pMeshAsset->m_indices.size() % 3 != 0u)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has indices which are not a multiple of 3!"));
		}

		for (unsigned int i = 0; i < pMeshAsset->m_vertices.size(); ++i)
		{
			const dx::XMFLOAT3 normal = (pMeshAsset->hasNormals) ? pMeshAsset->m_normals[i] : dx::XMFLOAT3(0, 0, 1);
			const dx::XMFLOAT4 tangent = (pMeshAsset->hasTangents) ? pMeshAsset->m_tangents[i] : dx::XMFLOAT4(0, 0, 1, 0);
			const dx::XMFLOAT4 vertColor = (pMeshAsset->hasVertColors) ? pMeshAsset->m_vertColors[i] : dx::XMFLOAT4(1, 1, 1, 1);
			const dx::XMFLOAT2 uv0 = (pMeshAsset->m_texcoords.size() > 0) ? pMeshAsset->m_texcoords[0][i] : dx::XMFLOAT2(0, 0);

			vbuf.EmplaceBack<dx::XMFLOAT3>(pMeshAsset->m_vertices[i]);
			vbuf.EmplaceBack<dx::XMFLOAT3>(normal);
			vbuf.EmplaceBack<dx::XMFLOAT4>(tangent);
			vbuf.EmplaceBack<dx::XMFLOAT4>(vertColor);
			vbuf.EmplaceBack<dx::XMFLOAT2>(uv0);
			vbuf.EmplacePadding();
		}

		return vbuf;
	}

	void BaseModel::ApplyTRS()
	{
		GameObject::ApplyTRS();
		RebuildSceneGraphTransforms();
	}

	void BaseModel::RebuildSceneGraphTransforms()
	{
		m_pSceneGraph->RebuildTransform(dx::XMLoadFloat4x4(&m_transform));
	}

	void BaseModel::InitializeModel()
	{
		m_pSceneGraph->RebuildBoundingVolumeHierarchy();
		RebuildSceneGraphTransforms();
	}

	void BaseModel::VerifyInstancing(const bool requireInstancing) const
	{
		for (const auto& m : m_pMaterials)
		{
			m->VerifyInstancing(requireInstancing);
		}
	}

	const std::shared_ptr<SceneGraphNode> BaseModel::GetSceneGraph() const
	{
		return m_pSceneGraph;
	}
}