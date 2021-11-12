#include "ModelInstance.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "PixelConstantBuffer.h"
#include "ModelAsset.h"
#include "VertexInclude.h"
#include <exception>
#include <assert.h>
//#include "Sphere.h"

namespace dx = DirectX;

ModelInstance::ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, std::string materialPath, dx::XMMATRIX transform)
	: transform(transform) // todo: set position
{
	std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, materialPath.c_str()));
	pMaterials.push_back(pMaterial);

	pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset->pSceneGraph);
}

ModelInstance::ModelInstance(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAsset, std::vector<std::string> materialPaths, dx::XMMATRIX transform)
	: transform(transform) // todo: set position
{
	pMaterials.reserve(materialPaths.size());
	for (const auto& path : materialPaths)
	{
		std::shared_ptr<Material> pMaterial = std::dynamic_pointer_cast<Material>(Material::Resolve(gfx, path.c_str()));
		pMaterials.push_back(pMaterial);
	}

	pSceneGraph = CreateModelInstanceNode(gfx, pModelAsset->pSceneGraph);
}

void ModelInstance::SubmitDrawCalls(FrameCommander& frame) const
{
	pSceneGraph->SubmitDrawCalls(frame, transform);
}

void ModelInstance::SetPositionWS(DirectX::XMFLOAT3 positionWS)
{
	transform = DirectX::XMMatrixTranslation(positionWS.x, positionWS.y, positionWS.z);
}

static int nextNodeId = 0; // todo: move
std::unique_ptr<Node> ModelInstance::CreateModelInstanceNode(Graphics& gfx, std::unique_ptr<SceneGraphNode<MeshAsset>> const& pSourceNode)
{
	// Copy mesh if needed
	auto pMeshInstance = std::unique_ptr<MeshRenderer>();
	if (pSourceNode->pMesh)
	{
		pMeshInstance = ParseMesh(gfx, pSourceNode->pMesh);
	}

	auto pChildNodes = std::vector<std::unique_ptr<Node>>();
	for (const auto& child : pSourceNode->pChildNodes)
	{
		auto pChildNode = CreateModelInstanceNode(gfx, child);
		pChildNodes.emplace_back(std::move(pChildNode));
	}
	/*for (size_t i = 0; i < pSourceNode->pChildNodes.size(); ++i)
	{
		auto pChildNode = CreateModelInstanceNode(gfx, pSourceNode->pChildNodes[i]);
		//pChildNodes.emplace_back(std::move(pChildNode));
	}*/

	auto pNode = std::make_unique<Node>(nextNodeId++, pSourceNode->localTransform, std::move(pMeshInstance), std::move(pChildNodes));
	return std::move(pNode);
}

std::unique_ptr<MeshRenderer> ModelInstance::ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset)
{
	namespace dx = DirectX;
	//using VertexLayout;

	/*VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Tangent)
		.Append(VertexLayout::Texture2D)*/

	auto pMaterial = pMaterials[0]; // todo: select via mesh

	VertexBufferData vbuf(pMaterial->GetVertexLayout());

	if (pMeshAsset->vertices.size() == 0)
	{
		throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has 0 vertices!"));
	}

	//if (pMeshAsset->hasTangents)
	//	gfx.log.Info(std::string("Mesh ") + pMeshAsset->name + std::string(" has tangents"));

	for (unsigned int i = 0; i < pMeshAsset->vertices.size(); ++i)
	{
		dx::XMFLOAT3 normal = (pMeshAsset->hasNormals) ? pMeshAsset->normals[i] : dx::XMFLOAT3(0, 0, 1);
		dx::XMFLOAT3 tangent = (pMeshAsset->hasTangents) ? pMeshAsset->tangents[i] : dx::XMFLOAT3(0, 0, 1);
		dx::XMFLOAT2 uv0 = (pMeshAsset->texcoords.size() > 0) ? pMeshAsset->texcoords[0][i] : dx::XMFLOAT2(0, 0);

		vbuf.EmplaceBack(
			pMeshAsset->vertices[i],
			normal,
			tangent,
			uv0
		);
		/*vbuf.EmplaceBack(
			*reinterpret_cast<dx::XMFLOAT3*>(&pMeshAsset->vertices[i]),
			*reinterpret_cast<dx::XMFLOAT3*>(&pMeshAsset->normals[i])
		);*/
	}

	// todo: better way to copy this?
	if (pMeshAsset->indices.size() == 0)
	{
		throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has 0 indices!"));
	}
	if (pMeshAsset->indices.size() % 3 != 0)
	{
		throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has indices which are not a multiple of 3!"));
	}
	std::vector<unsigned short> indices;
	indices.reserve(pMeshAsset->indices.size());
	for (unsigned int i = 0; i < pMeshAsset->indices.size(); ++i)
	{
		indices.push_back(pMeshAsset->indices[i]);
	}

	auto meshTag = "Mesh%" + pMeshAsset->name;
	std::shared_ptr<VertexBuffer> pVertexBuffer = VertexBuffer::Resolve(gfx, meshTag, vbuf); // vbuf is passed as const&
	std::shared_ptr<IndexBuffer> pIndexBuffer = IndexBuffer::Resolve(gfx, meshTag, indices);
	std::shared_ptr<Topology> pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return std::make_unique<MeshRenderer>(gfx, pMeshAsset->name, pMaterial, std::move(pVertexBuffer), std::move(pIndexBuffer), std::move(pTopology));
}

Node::Node(int id, const DirectX::XMMATRIX & _transform, std::unique_ptr<MeshRenderer> pMeshPtr, std::vector<std::unique_ptr<Node>> pChildNodes)
	: pMeshPtr(std::move(pMeshPtr)), pChildNodes(std::move(pChildNodes))
{
	dx::XMStoreFloat4x4(&transform, _transform);
	dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}

void Node::SubmitDrawCalls(FrameCommander& frame, DirectX::FXMMATRIX accumulatedTransform) const
{
	// todo: use this?
	const auto built =
		dx::XMLoadFloat4x4(&appliedTransform) *
		dx::XMLoadFloat4x4(&transform) *
		accumulatedTransform;

	if (pMeshPtr)
	{
		pMeshPtr->SubmitDrawCalls(frame, built);
	}

	for (const auto& pc : pChildNodes)
	{
		pc->SubmitDrawCalls(frame, built);
	}
}

void Node::SetAppliedTransform(DirectX::FXMMATRIX _transform) noexcept
{
	dx::XMStoreFloat4x4(&appliedTransform, _transform);
}

const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
{
	return appliedTransform;
}
