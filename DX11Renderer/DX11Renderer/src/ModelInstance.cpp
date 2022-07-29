#include "pch.h"
#include "ModelInstance.h"
#include "GraphicsDevice.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "MeshRenderer.h"
#include "VertexBufferWrapper.h"
#include "VertexAttributesLayout.h"
#include "IndexBuffer.h"
#include "Topology.h"
#include <exception>
#include <assert.h>
#include "InstancedMeshRenderer.h"
#include "RawBufferData.h"
#include "StructuredBufferData.h"
#include "DrawContext.h"

namespace gfx
{
	ModelInstance::ModelInstance(const GraphicsDevice& gfx, const ModelAsset& pModelAsset, const dx::XMMATRIX transform)
		: ModelInstance(gfx, pModelAsset, transform, dx::XMVectorSet(1.f, 1.f, 1.f, 1.f))
	{}

	ModelInstance::ModelInstance(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX transform)
		: ModelInstance(gfx, pModelAsset, transform, dx::XMVectorSet(1.f, 1.f, 1.f, 1.f))
	{}

	ModelInstance::ModelInstance(const GraphicsDevice& gfx, const ModelAsset& pModelAsset, const dx::XMMATRIX transform, const dx::XMVECTOR scale)
		: m_transform(transform), m_scale(scale)
	{
		m_pMaterials.reserve(pModelAsset.m_materialPaths.size());
		for (const auto& materialPath : pModelAsset.m_materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
			m_pMaterials.push_back(pMaterial);
		}

		m_pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset.m_pSceneGraph);
		InitializeModel();
	}

	ModelInstance::ModelInstance(const GraphicsDevice& gfx, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMMATRIX transform, const dx::XMVECTOR scale)
		: m_transform(transform), m_scale(scale)
	{
		m_pMaterials.reserve(pModelAsset->m_materialPaths.size());
		for (const auto& materialPath : pModelAsset->m_materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
			m_pMaterials.push_back(pMaterial);
		}

		m_pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset->m_pSceneGraph);
		InitializeModel();
	}

	void ModelInstance::InitializeModel()
	{
		m_pSceneGraph->RebuildBoundingVolumeHierarchy();
		RebuildSceneGraphTransforms();
	}

	/*void ModelInstance::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		m_pSceneGraph->SubmitDrawCalls(drawContext);
	}*/

	void ModelInstance::SetPositionWS(const dx::XMFLOAT3 positionWS)
	{
		m_transform = dx::XMMatrixScalingFromVector(m_scale)
			* dx::XMMatrixTranslation(positionWS.x, positionWS.y, positionWS.z);
		RebuildSceneGraphTransforms();
	}

	void ModelInstance::RebuildSceneGraphTransforms()
	{
		m_pSceneGraph->RebuildTransform(m_transform);
	}

	const std::shared_ptr<SceneGraphNode> ModelInstance::GetSceneGraph() const
	{
		return m_pSceneGraph;
	}

	const std::vector<std::shared_ptr<MeshRenderer>>& ModelInstance::GetMeshRenderers() const
	{
		return m_pMeshes;
	}

	static int nextNodeId = 0; // todo: move
	std::shared_ptr<SceneGraphNode> ModelInstance::CreateModelInstanceNode(const GraphicsDevice& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode)
	{
		gfx.GetLog().Info("Create ModelInstanceNode " + pSourceNode->m_name + " w/ " + std::to_string(pSourceNode->m_pChildNodes.size()) + " children");

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
			auto pChildNode = CreateModelInstanceNode(gfx, child);
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

	std::shared_ptr<MeshRenderer> ModelInstance::CreateMeshRenderer(const GraphicsDevice& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset)
	{
		// temporary way of testing instances
		const bool isInstance = false;
		const UINT instanceCount = 10u;

		const auto pMaterial = m_pMaterials[pMeshAsset->m_materialIndex];

		if (pMeshAsset->m_vertices.size() == 0)
		{
			THROW(std::string("Mesh '") + pMeshAsset->m_name + std::string("' has 0 vertices!"));
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

		const auto meshTag = "Mesh%" + pMeshAsset->m_name;

		// Create a unique vertex buffer for each attributes slot
		std::vector<std::shared_ptr<VertexBufferWrapper>> pVertexBuffers;
		for (size_t attribIdx = 0; attribIdx < pMaterial->GetAttributeSlotsCount(); ++attribIdx)
		{
			// Buffer data prep
			RawBufferData vbuf(pMeshAsset->m_vertices.size(), pMaterial->GetVertexLayout(attribIdx).GetPerVertexStride(), pMaterial->GetVertexLayout(attribIdx).GetPerVertexPadding());
			for (unsigned int i = 0; i < pMeshAsset->m_vertices.size(); ++i)
			{
				// todo: optimize!
				if (pMaterial->GetVertexLayout(attribIdx).HasPosition())
					vbuf.EmplaceBack<dx::XMFLOAT3>(pMeshAsset->GetVertexPosition(i));
				if (pMaterial->GetVertexLayout(attribIdx).HasNormal())
					vbuf.EmplaceBack<dx::XMFLOAT3>(pMeshAsset->GetNormalOrDefault(i));
				if (pMaterial->GetVertexLayout(attribIdx).HasTangent())
					vbuf.EmplaceBack<dx::XMFLOAT4>(pMeshAsset->GetTangentOrDefault(i));
				if (pMaterial->GetVertexLayout(attribIdx).HasTexcoord(0))
					vbuf.EmplaceBack<dx::XMFLOAT2>(pMeshAsset->GetTexcoordOrDefault(i, 0));
				vbuf.EmplacePadding();
			}

			// Buffer creation
			if (isInstance)
			{
				StructuredBufferData<InstanceData> instanceBuf(instanceCount);
				for (size_t i = 0; i < instanceCount; ++i)
				{
					instanceBuf.EmplaceBack(InstanceData{ dx::XMFLOAT3(i, 0, 0), (UINT)i });
				}
				pVertexBuffers.emplace_back(VertexBufferWrapper::Resolve(gfx, meshTag, vbuf, instanceBuf));
			}
			else
			{
				pVertexBuffers.emplace_back(VertexBufferWrapper::Resolve(gfx, meshTag, vbuf));
			}
		}
		
		// todo: find better way to copy this?
		std::vector<u32> indices;
		indices.reserve(pMeshAsset->GetIndexCount());
		for (unsigned int i = 0; i < pMeshAsset->GetIndexCount(); ++i)
		{
			indices.push_back(pMeshAsset->GetIndex(i));
		}
		
		std::shared_ptr<IndexBuffer> pIndexBuffer = IndexBuffer::Resolve(gfx, meshTag, indices);
		std::shared_ptr<Topology> pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (isInstance)
		{
			return std::make_shared<InstancedMeshRenderer>(gfx, pMeshAsset->m_name, pMeshAsset, pMaterial, std::move(pVertexBuffers), std::move(pIndexBuffer), std::move(pTopology), instanceCount);
		}
		else
		{
			return std::make_shared<MeshRenderer>(gfx, pMeshAsset->m_name, pMeshAsset, pMaterial, std::move(pVertexBuffers), std::move(pIndexBuffer), std::move(pTopology));
		}
	}
}