#include "pch.h"
#include "Model.h"
#include "GraphicsDevice.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "MeshRenderer.h"
#include "Material.h"
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

	/*const std::vector<std::shared_ptr<MeshRenderer>>& Model::GetMeshRenderers() const
	{
		return m_pMeshRenderers;
	}*/

	std::shared_ptr<SceneGraphNode> Model::CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode)
	{
		gfx.GetLog().Info("Create ModelNode " + pSourceNode->m_name + " w/ " + std::to_string(pSourceNode->m_pChildNodes.size()) + " children");

		// Copy mesh if needed
		auto pMeshRenderer = std::shared_ptr<MeshRenderer>();
		if (pSourceNode->m_pMeshAsset)
		{
			pMeshRenderer = CreateMeshRenderer(gfx, pSourceNode->m_pMeshAsset);
			m_pMeshRenderers.emplace_back(pMeshRenderer);
		}

		auto pChildNodes = std::vector<std::shared_ptr<SceneGraphNode>>();
		for (const auto& child : pSourceNode->m_pChildNodes)
		{
			auto pChildNode = CreateModelNode(gfx, child);
			pChildNodes.emplace_back(std::move(pChildNode));
		}

		auto pNode = std::make_shared<SceneGraphNode>(dx::XMLoadFloat4x4(&pSourceNode->m_localTransform), pMeshRenderer, std::move(pChildNodes));

		// After creating, set parent
		for (const auto& child : pNode->m_pChildNodes)
		{
			child->m_pParentNode = pNode;
		}
		
		return std::move(pNode);
	}

	std::shared_ptr<MeshRenderer> Model::CreateMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset)
	{
		const auto meshTag = "Mesh%" + pMeshAsset->m_name;
		const auto pMaterial = m_pMaterials.at(pMeshAsset->m_materialIndex);
		const auto vbuf = CreateVertexBufferData(pMeshAsset, pMaterial);

		std::shared_ptr<VertexBufferWrapper> pVertexBuffer = VertexBufferWrapper::Resolve(gfx, meshTag, vbuf);
		std::shared_ptr<IndexBuffer> pIndexBuffer = IndexBuffer::Resolve(gfx, meshTag, pMeshAsset->m_indices);
		std::shared_ptr<Topology> pTopology = Topology::Resolve(gfx, pMaterial->HasTessellation() ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		return std::make_shared<MeshRenderer>(gfx, pMeshAsset->m_name, pMeshAsset, pMaterial, std::move(pVertexBuffer), std::move(pIndexBuffer), std::move(pTopology));
	}
}