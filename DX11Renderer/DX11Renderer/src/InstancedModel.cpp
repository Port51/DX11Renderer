#include "pch.h"
#include "InstancedModel.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "MeshRenderer.h"
#include "InstancedMeshRenderer.h"
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
#include "RandomGenerator.h"

namespace gfx
{

	InstancedModel::InstancedModel(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX& transform, const std::vector<TRS>& instancePositions, const RandomGenerator& randomGenerator)
		: m_instanceCount(instancePositions.size())
	{
		dx::XMStoreFloat4x4(&m_transform, transform);
		DecomposeTRS();

		m_pMaterials.reserve(pModelAsset->m_materialPaths.size());
		for (const auto& materialPath : pModelAsset->m_materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str(), true));
			m_pMaterials.push_back(pMaterial);
		}

		VerifyInstancing(true);

		// Setup initial data
		m_pInstanceBuf = std::make_unique<StructuredBufferData<InstanceData>>(m_instanceCount);
		for (size_t i = 0; i < m_instanceCount; ++i)
		{
			m_pInstanceBuf->EmplaceBack(InstanceData{ instancePositions.at(i).trs, dx::XMFLOAT4(1, 1, 1, 1), dx::XMFLOAT4(randomGenerator.GetUniformFloat01(), randomGenerator.GetUniformFloat01(), randomGenerator.GetUniformFloat01(), static_cast<float>(i))});
		}

		m_pSceneGraph = CreateModelNode(gfx, pModelAsset->m_pSceneGraph, instancePositions);
		InitializeModel();
	}

	InstancedModel::~InstancedModel()
	{}

	const size_t InstancedModel::GetInstanceCount() const
	{
		return m_instanceCount;
	}

	StructuredBufferData<InstanceData>& InstancedModel::GetInstanceData() const
	{
		return *m_pInstanceBuf;
	}

	InstanceData InstancedModel::GetInstanceDataPoint(const size_t i) const
	{
		return m_pInstanceBuf->GetElement(i);
	}

	void InstancedModel::SetInstanceDataPoint(const size_t i, const InstanceData& data)
	{
		m_pInstanceBuf->SetElement(i, data);
	}

	void InstancedModel::ApplyInstanceData(const GraphicsDevice& gfx)
	{
		for (const auto& vb : m_pVertexBuffers)
		{
			vb->ApplyInstanceData(gfx, *m_pInstanceBuf);
		}
	}

	/*const std::vector<std::shared_ptr<MeshRenderer>>& InstancedModel::GetMeshRenderers() const
	{
		return m_pMeshRenderers;
	}*/

	std::shared_ptr<SceneGraphNode> InstancedModel::CreateModelNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode, const std::vector<TRS>& instancePositions)
	{
		gfx.GetLog().Info("Create ModelNode " + pSourceNode->m_name + " w/ " + std::to_string(pSourceNode->m_pChildNodes.size()) + " children");

		// Copy mesh if needed
		auto pMeshRenderer = std::shared_ptr<InstancedMeshRenderer>();
		if (pSourceNode->m_pMeshAsset)
		{
			pMeshRenderer = CreateInstancedMeshRenderer(gfx, pSourceNode->m_pMeshAsset, instancePositions);
			m_pMeshRenderers.emplace_back(pMeshRenderer);
		}

		auto pChildNodes = std::vector<std::shared_ptr<SceneGraphNode>>();
		for (const auto& child : pSourceNode->m_pChildNodes)
		{
			auto pChildNode = CreateModelNode(gfx, child, instancePositions);
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

	std::shared_ptr<InstancedMeshRenderer> InstancedModel::CreateInstancedMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset, const std::vector<TRS>& instancePositions)
	{
		const auto meshTag = "Mesh%" + pMeshAsset->m_name;
		const auto pMaterial = m_pMaterials.at(pMeshAsset->m_materialIndex);
		const auto vbuf = CreateVertexBufferData(pMeshAsset, pMaterial);

		std::shared_ptr<VertexBufferWrapper> pVertexBuffer = std::make_shared<VertexBufferWrapper>(gfx, vbuf, *m_pInstanceBuf);
		std::shared_ptr<IndexBuffer> pIndexBuffer = IndexBuffer::Resolve(gfx, meshTag, pMeshAsset->m_indices);
		std::shared_ptr<Topology> pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_pVertexBuffers.emplace_back(pVertexBuffer);

		return std::make_shared<InstancedMeshRenderer>(gfx, pMeshAsset->m_name, pMeshAsset, pMaterial, pVertexBuffer, std::move(pIndexBuffer), std::move(pTopology), m_instanceCount);
	}
}