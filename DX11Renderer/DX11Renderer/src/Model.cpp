#include "pch.h"
#include "Model.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "MeshRenderer.h"
#include "VertexBufferWrapper.h"
#include "IndexBuffer.h"
#include "Topology.h"
#include <exception>
#include <assert.h>
#include "InstancedMeshRenderer.h"
#include "RawBufferData.h"
#include "StructuredBufferData.h"
#include "DrawContext.h"
#include "SceneGraphNode.h"

namespace gfx
{
	Model::Model(const GraphicsDevice& gfx, const ModelAsset& pModelAsset, const dx::XMMATRIX& transform)
	{
		dx::XMStoreFloat4x4(&m_transform, transform);
		DecomposeTRS();

		m_pMaterials.reserve(pModelAsset.m_materialPaths.size());
		for (const auto& materialPath : pModelAsset.m_materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
			m_pMaterials.push_back(pMaterial);
		}

		m_pSceneGraph = CreateModelNode(gfx, pModelAsset.m_pSceneGraph);
		InitializeModel();
	}

	Model::Model(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform)
	{
		dx::XMStoreFloat4x4(&m_transform, transform);
		DecomposeTRS();

		m_pMaterials.reserve(pModelAsset->m_materialPaths.size());
		for (const auto& materialPath : pModelAsset->m_materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
			m_pMaterials.push_back(pMaterial);
		}

		m_pSceneGraph = CreateModelNode(gfx, pModelAsset->m_pSceneGraph);
		InitializeModel();
	}

	Model::~Model()
	{}

	void Model::InitializeModel()
	{
		m_pSceneGraph->RebuildBoundingVolumeHierarchy();
		RebuildSceneGraphTransforms();
	}

	void Model::ApplyTRS()
	{
		GameObject::ApplyTRS();
		RebuildSceneGraphTransforms();
	}

	void Model::RebuildSceneGraphTransforms()
	{
		m_pSceneGraph->RebuildTransform(dx::XMLoadFloat4x4(&m_transform));
	}

	const std::shared_ptr<SceneGraphNode> Model::GetSceneGraph() const
	{
		return m_pSceneGraph;
	}

	const std::vector<std::shared_ptr<MeshRenderer>>& Model::GetMeshRenderers() const
	{
		return m_pMeshes;
	}

	static int nextNodeId = 0; // todo: move
	std::shared_ptr<SceneGraphNode> Model::CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode)
	{
		gfx.GetLog().Info("Create ModelNode " + pSourceNode->m_name + " w/ " + std::to_string(pSourceNode->m_pChildNodes.size()) + " children");

		// Copy mesh if needed
		auto pMeshRenderer = std::shared_ptr<MeshRenderer>();
		if (pSourceNode->m_pMeshAsset)
		{
			pMeshRenderer = CreateMeshRenderer(gfx, pSourceNode->m_pMeshAsset);
			m_pMeshes.emplace_back(pMeshRenderer);
		}

		auto pChildNodes = std::vector<std::shared_ptr<SceneGraphNode>>();
		for (const auto& child : pSourceNode->m_pChildNodes)
		{
			auto pChildNode = CreateModelNode(gfx, child);
			pChildNodes.emplace_back(std::move(pChildNode));
		}

		auto pNode = std::make_shared<SceneGraphNode>(nextNodeId++, dx::XMLoadFloat4x4(&pSourceNode->m_localTransform), pMeshRenderer, std::move(pChildNodes));

		// After creating, set parent
		for (const auto& child : pNode->m_pChildNodes)
		{
			child->m_pParentNode = pNode;
		}
		
		return std::move(pNode);
	}

	std::shared_ptr<MeshRenderer> Model::CreateMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset)
	{
		// temporary way of testing instances
		const bool isInstance = false;
		const UINT instanceCount = 10u;

		const auto pMaterial = m_pMaterials.at(pMeshAsset->m_materialIndex);
		RawBufferData vbuf(pMeshAsset->m_vertices.size(), pMaterial->GetVertexLayout().GetPerVertexStride(), pMaterial->GetVertexLayout().GetPerVertexPadding());

		if (pMeshAsset->m_vertices.size() == 0)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has 0 vertices!"));
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

		if (pMeshAsset->m_indices.size() == 0)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has 0 indices!"));
		}
		if (pMeshAsset->m_indices.size() % 3 != 0)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has indices which are not a multiple of 3!"));
		}

		// todo: move instance stuff somewhere else!
		struct InstanceData
		{
			dx::XMFLOAT3 positionWS;
			UINT instanceId;
		};

		// todo: better way to copy this?
		std::vector<u32> indices;
		indices.reserve(pMeshAsset->m_indices.size());
		for (unsigned int i = 0; i < pMeshAsset->m_indices.size(); ++i)
		{
			indices.push_back(pMeshAsset->m_indices[i]);
		}

		const auto meshTag = "Mesh%" + pMeshAsset->m_name;

		std::shared_ptr<VertexBufferWrapper> pVertexBuffer;
		if (isInstance)
		{
			StructuredBufferData<InstanceData> instanceBuf(instanceCount);
			for (size_t i = 0; i < instanceCount; ++i)
			{
				instanceBuf.EmplaceBack(InstanceData{ dx::XMFLOAT3(i, 0, 0), (UINT)i });
			}
			pVertexBuffer = VertexBufferWrapper::Resolve(gfx, meshTag, vbuf, instanceBuf);
		}
		else
		{
			pVertexBuffer = VertexBufferWrapper::Resolve(gfx, meshTag, vbuf);
		}
		
		std::shared_ptr<IndexBuffer> pIndexBuffer = IndexBuffer::Resolve(gfx, meshTag, indices);
		std::shared_ptr<Topology> pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (isInstance)
		{
			return std::make_shared<InstancedMeshRenderer>(gfx, pMeshAsset->m_name, pMeshAsset, pMaterial, std::move(pVertexBuffer), std::move(pIndexBuffer), std::move(pTopology), instanceCount);
		}
		else
		{
			return std::make_shared<MeshRenderer>(gfx, pMeshAsset->m_name, pMeshAsset, pMaterial, std::move(pVertexBuffer), std::move(pIndexBuffer), std::move(pTopology));
		}
	}
}