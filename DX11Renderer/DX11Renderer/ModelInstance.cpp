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

void ModelInstance::Draw(Graphics& gfx) const
{
	pSceneGraph->Draw(gfx, transform);
}

void ModelInstance::SetPositionWS(DirectX::XMFLOAT3 positionWS)
{
	transform = DirectX::XMMatrixTranslation(positionWS.x, positionWS.y, positionWS.z);
}

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

	auto pNode = std::make_unique<Node>(pSourceNode->localTransform, std::move(pMeshInstance), std::move(pChildNodes));
	return std::move(pNode);
}

std::unique_ptr<MeshRenderer> ModelInstance::ParseMesh(Graphics& gfx, std::unique_ptr<MeshAsset> const& pMeshAsset)
{
	namespace dx = DirectX;
	//using VertexLayout;

	VertexBufferData vbuf(std::move(
		VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Texture2D)
	));

	if (pMeshAsset->vertices.size() == 0)
	{
		throw std::runtime_error(std::string("Mesh '") + pMeshAsset->name + std::string("' has 0 vertices!"));
	}
	for (unsigned int i = 0; i < pMeshAsset->vertices.size(); ++i)
	{
		dx::XMFLOAT3 normals = (pMeshAsset->hasNormals) ? pMeshAsset->normals[i] : dx::XMFLOAT3(0, 0, 1);
		dx::XMFLOAT2 uv0 = (pMeshAsset->texcoords.size() > 0) ? pMeshAsset->texcoords[0][i] : dx::XMFLOAT2(0, 0);

		vbuf.EmplaceBack(
			pMeshAsset->vertices[i],
			normals,
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

	std::vector<std::shared_ptr<Bindable>> pBindablePtrs;

	auto meshTag = "Mesh%" + pMeshAsset->name;
	pBindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf)); // vbuf is passed as const&
	pBindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

	auto pMaterial = pMaterials[0]; // todo: select via mesh
	pBindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pMaterial->pVertexShader->GetBytecode()));

	return std::make_unique<MeshRenderer>(gfx, pMeshAsset->name, pMaterial, std::move(pBindablePtrs));
}

void Node::Draw(Graphics & gfx, DirectX::FXMMATRIX accumulatedTransform) const
{
	auto combinedTransform =
		//dx::XMLoadFloat4x4(&appliedTransform) *
		dx::XMLoadFloat4x4(&localTransform) *
		accumulatedTransform;

	if (pMeshPtr)
	{
		pMeshPtr->Draw(gfx, combinedTransform);
	}
	for (const auto& pc : pChildNodes)
	{
		pc->Draw(gfx, combinedTransform);
	}
}
