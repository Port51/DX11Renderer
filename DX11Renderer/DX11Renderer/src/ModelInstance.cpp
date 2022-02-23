#include "pch.h"
#include "ModelInstance.h"
#include "BindableInclude.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include <exception>
#include <assert.h>
#include "InstancedMeshRenderer.h"
#include "RawBufferData.h"
#include "StructuredBufferData.h"
#include "DrawContext.h"

namespace gfx
{
	ModelInstance::ModelInstance(Graphics& gfx, const ModelAsset& pModelAsset, dx::XMMATRIX transform)
		: transform(transform)
	{
		pMaterials.reserve(pModelAsset.materialPaths.size());
		for (const auto& materialPath : pModelAsset.materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
			pMaterials.push_back(pMaterial);
		}

		pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset.pSceneGraph);
		UpdateSceneGraph();
	}

	ModelInstance::ModelInstance(Graphics & gfx, std::shared_ptr<ModelAsset> pModelAsset, dx::XMMATRIX transform)
		: transform(transform)
	{
		pMaterials.reserve(pModelAsset->materialPaths.size());
		for (const auto& materialPath : pModelAsset->materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
			pMaterials.push_back(pMaterial);
		}

		pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset->pSceneGraph);
		UpdateSceneGraph();
	}

	/*ModelInstance::ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, std::vector<std::string> materialPaths, dx::XMMATRIX transform)
		: transform(transform) // todo: set position
	{
		pMaterials.reserve(materialPaths.size());
		for (const auto& path : materialPaths)
		{
			std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, path.c_str()));
			pMaterials.push_back(pMaterial);
		}

		pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset->pSceneGraph);
	}*/

	void ModelInstance::SubmitDrawCalls(const DrawContext& drawContext) const
	{
		pSceneGraph->SubmitDrawCalls(drawContext);
	}

	void ModelInstance::SetPositionWS(dx::XMFLOAT3 positionWS)
	{
		transform = dx::XMMatrixTranslation(positionWS.x, positionWS.y, positionWS.z);
		UpdateSceneGraph();
	}

	void ModelInstance::UpdateSceneGraph()
	{
		pSceneGraph->RebuildTransform(transform);
	}

	std::vector<std::shared_ptr<MeshRenderer>> ModelInstance::GetMeshRenderers() const
	{
		return pMeshes;
	}

	static int nextNodeId = 0; // todo: move
	std::shared_ptr<SceneGraphNode> ModelInstance::CreateModelInstanceNode(Graphics& gfx, std::shared_ptr<ModelAssetNode> const& pSourceNode)
	{
		// Copy mesh if needed
		auto pMeshRenderer = std::shared_ptr<MeshRenderer>();
		if (pSourceNode->pMeshAsset)
		{
			pMeshRenderer = CreateMeshRenderer(gfx, pSourceNode->pMeshAsset);
			pMeshes.emplace_back(pMeshRenderer);
		}

		auto pChildNodes = std::vector<std::shared_ptr<SceneGraphNode>>();
		for (const auto& child : pSourceNode->pChildNodes)
		{
			auto pChildNode = CreateModelInstanceNode(gfx, child);
			pChildNodes.emplace_back(std::move(pChildNode));
		}
		/*for (UINT i = 0; i < pSourceNode->pChildNodes.size(); ++i)
		{
			auto pChildNode = CreateModelInstanceNode(gfx, pSourceNode->pChildNodes[i]);
			//pChildNodes.emplace_back(std::move(pChildNode));
		}*/

		auto pNode = std::make_unique<SceneGraphNode>(nextNodeId++, dx::XMLoadFloat4x4(&pSourceNode->localTransform), pMeshRenderer, std::move(pChildNodes));
		return std::move(pNode);
	}

	std::shared_ptr<MeshRenderer> ModelInstance::CreateMeshRenderer(Graphics& gfx, std::shared_ptr<MeshAsset> const& pMeshAsset)
	{
		const auto pMaterial = pMaterials[pMeshAsset->materialIndex];
		RawBufferData vbuf(pMeshAsset->vertices.size(), pMaterial->GetVertexLayout().GetPerVertexStride(), pMaterial->GetVertexLayout().GetPerVertexPadding());

		if (pMeshAsset->vertices.size() == 0)
		{
			throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has 0 vertices!"));
		}

		for (unsigned int i = 0; i < pMeshAsset->vertices.size(); ++i)
		{
			dx::XMFLOAT3 normal = (pMeshAsset->hasNormals) ? pMeshAsset->normals[i] : dx::XMFLOAT3(0, 0, 1);
			dx::XMFLOAT3 tangent = (pMeshAsset->hasTangents) ? pMeshAsset->tangents[i] : dx::XMFLOAT3(0, 0, 1);
			dx::XMFLOAT2 uv0 = (pMeshAsset->texcoords.size() > 0) ? pMeshAsset->texcoords[0][i] : dx::XMFLOAT2(0, 0);

			vbuf.EmplaceBack<dx::XMFLOAT3>(pMeshAsset->vertices[i]);
			vbuf.EmplaceBack<dx::XMFLOAT3>(normal);
			vbuf.EmplaceBack<dx::XMFLOAT3>(tangent);
			vbuf.EmplaceBack<dx::XMFLOAT2>(uv0);
			vbuf.EmplacePadding();
		}

		if (pMeshAsset->indices.size() == 0)
		{
			throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has 0 indices!"));
		}
		if (pMeshAsset->indices.size() % 3 != 0)
		{
			throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has indices which are not a multiple of 3!"));
		}

		// todo: move instance stuff somewhere else!
		struct InstanceData
		{
			dx::XMFLOAT3 positionWS;
			float padding;
		};

		const UINT instanceCount = 10u;
		//InstanceData* instances = new InstanceData[instanceCount];
		StructuredBufferData<InstanceData> instanceBuf(instanceCount);
		for (int i = 0; i < instanceCount; ++i)
		{
			instanceBuf.EmplaceBack(InstanceData{ dx::XMFLOAT3(i, 0, 0), 0.f });
		}

		// Release the instance array now that the instance buffer has been created and loaded.
		//delete[] instances;

		// todo: better way to copy this?
		std::vector<u32> indices;
		indices.reserve(pMeshAsset->indices.size());
		for (unsigned int i = 0; i < pMeshAsset->indices.size(); ++i)
		{
			indices.push_back(pMeshAsset->indices[i]);
		}

		auto meshTag = "Mesh%" + pMeshAsset->name;
		std::unique_ptr<VertexBufferWrapper> pVertexBuffer = std::make_unique<VertexBufferWrapper>(gfx, vbuf, instanceBuf);
		std::shared_ptr<IndexBuffer> pIndexBuffer = IndexBuffer::Resolve(gfx, meshTag, indices);
		std::shared_ptr<Topology> pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// temporary
		const bool IsInstance = false;
		if (IsInstance)
			return std::make_shared<InstancedMeshRenderer>(gfx, pMeshAsset->name, pMaterial, std::move(pVertexBuffer), std::move(pIndexBuffer), std::move(pTopology), instanceCount);
		else
			return std::make_shared<MeshRenderer>(gfx, pMeshAsset->name, pMaterial, std::move(pVertexBuffer), std::move(pIndexBuffer), std::move(pTopology));
	}
}