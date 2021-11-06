#pragma once
#include <fbxsdk.h>
#include <fbxgeometryconverter.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <exception>
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "SceneGraphNode.h"
#include "GraphicsThrowMacros.h"

// Blender export settings:
// - Use "FBX Units Scale"
// - Apply Unit = OFF

namespace dx = DirectX;

// Memory manager object
static FbxManager* fbxSdkManager = nullptr;

class FBXImporter
{
public:
	enum FBXNormalsMode
	{
		Import,
		FlatShaded,
	};

	static std::unique_ptr<ModelAsset> LoadFBX(const char* filename, FBXNormalsMode normalsMode, bool triangulate = true)
	{
		// Refs:
		// * http://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_getting_started_your_first_fbx_sdk_program_html

		std::unique_ptr<ModelAsset> pOutputModel;

		// Init memory manager if needed
		if (fbxSdkManager == nullptr)
		{
			fbxSdkManager = FbxManager::Create();

			FbxIOSettings* pIOsettings = FbxIOSettings::Create(fbxSdkManager, IOSROOT);
			fbxSdkManager->SetIOSettings(pIOsettings);
		}

		FbxImporter* pImporter = FbxImporter::Create(fbxSdkManager, "");
		if (pImporter->Initialize(filename, -1, fbxSdkManager->GetIOSettings()))
		{
			FbxScene* pFbxScene = FbxScene::Create(fbxSdkManager, "");
			if (pImporter->Import(pFbxScene))
			{
				pImporter->Destroy(); // no need to keep this around

				if (triangulate)
				{
					FbxGeometryConverter clsConverter(fbxSdkManager);
					clsConverter.Triangulate(pFbxScene, true);
				}

				// The root node should not contain any attributes
				FbxNode* pFbxRootNode = pFbxScene->GetRootNode();
				if (pFbxRootNode)
				{
					auto pSceneGraph = UnpackFbxSceneGraph(pFbxRootNode, normalsMode);
					auto pModelAsset = std::make_unique<ModelAsset>(std::move(pSceneGraph));

					return std::move(pModelAsset);
				}
				else
				{
					throw std::runtime_error("No root node was found in FBX for filename '" + std::string(filename) + "'");
				}
			}
			else
			{
				throw std::runtime_error("Could not import FBX scene for filename '" + std::string(filename) + "'");
			}
		}
		else
		{
			throw std::runtime_error("Could not initialize FBX importer for filename '" + std::string(filename) + "'");
		}

		pImporter->Destroy();
		return nullptr;
	}

	// Recursively unpack scene graph and return root node
	//static std::unique_ptr<SceneGraphNode<MeshAsset>> UnpackFbxSceneGraph(std::unique_ptr<FbxNode> pFbxNode)
	static std::unique_ptr<SceneGraphNode<MeshAsset>> UnpackFbxSceneGraph(FbxNode* pFbxNode, FBXNormalsMode normalsMode)
	{
		const auto name = pFbxNode->GetName();

		//
		// Unpack transform
		//
		const auto fbxTransl = pFbxNode->LclTranslation.Get();
		const auto fbxRot = pFbxNode->LclRotation.Get();
		const auto fbxScale = pFbxNode->LclScaling.Get();
		const auto localTransform =
			dx::XMMatrixScaling(fbxScale[0], fbxScale[1], fbxScale[2])
			* dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(fbxRot[0]), dx::XMConvertToRadians(fbxRot[1]), dx::XMConvertToRadians(fbxRot[2])) // untested!
			* dx::XMMatrixTranslation(fbxTransl[0], fbxTransl[1], fbxTransl[2]);
		assert("Incorrect node scale - was FBX exported with 'FBX Units Scale' option?" && fbxScale[0] < 9999.f);

		//
		// Unpack mesh
		//
		auto pMeshAsset = std::unique_ptr<MeshAsset>();
		if (pFbxNode->GetNodeAttribute())
		{
			FbxNodeAttribute::EType attributeType = pFbxNode->GetNodeAttribute()->GetAttributeType();
			if (attributeType == FbxNodeAttribute::eMesh)
			{
				// Import mesh
				//auto pFbxMesh = std::make_unique<FbxMesh>((FbxMesh*)pFbxNode->GetNodeAttribute());
				//pMeshAsset = LoadFbxMesh(std::move(pFbxMesh));
				FbxMesh* pFbxMesh = (FbxMesh*)pFbxNode->GetNodeAttribute();
				pMeshAsset = LoadFbxMesh(std::string(pFbxNode->GetName()), pFbxMesh, normalsMode);
			}
		}

		//
		// Iterate through children
		//
		auto pChildNodes = std::vector<std::unique_ptr<SceneGraphNode<MeshAsset>>>();
		for (int i = 0; i < pFbxNode->GetChildCount(); i++)
		{
			//auto pFbxChildNode = std::make_unique<FbxNode>(pFbxNode->GetChild(i));
			FbxNode* pFbxChildNode = pFbxNode->GetChild(i);
			if (pFbxChildNode->GetNodeAttribute())
			{
				//pChildNodes.emplace_back(UnpackFbxSceneGraph(std::move(pFbxChildNode)));
				pChildNodes.emplace_back(UnpackFbxSceneGraph(pFbxChildNode, normalsMode));
			}
		}

		auto pResult
			= std::make_unique<SceneGraphNode<MeshAsset>>(localTransform, std::move(pMeshAsset), std::move(pChildNodes));
		return std::move(pResult);
	}

	//static std::unique_ptr<MeshAsset> LoadFbxMesh(std::unique_ptr<FbxMesh> pFbxMesh)
	static std::unique_ptr<MeshAsset> LoadFbxMesh(std::string name, FbxMesh* pFbxMesh, FBXNormalsMode normalsMode)
	{
		auto pMesh = std::make_unique<MeshAsset>();
		pMesh->name = name;

		FbxVector4* pFbxVertices = pFbxMesh->GetControlPoints();

		// To convert to XMFLOAT3, use this:
		// *reinterpret_cast<dx::XMFLOAT3*>(&pMesh->mNormals[i])

		pMesh->vertices = std::move(GetFbxVertices(pFbxMesh));

		int* pIndices = pFbxMesh->GetPolygonVertices();

		// Indices
		for (int j = 0; j < pFbxMesh->GetPolygonVertexCount(); ++j)
		{
			pMesh->indices.push_back(pIndices[j]);
		}

		switch (normalsMode)
		{
		case FBXNormalsMode::Import:
			pMesh->normals = std::move(GetFbxNormals(pFbxMesh));
			pMesh->tangents = std::move(GetFbxTangents(pFbxMesh));
			break;
		case FBXNormalsMode::FlatShaded:
			pMesh->normals = std::move(GetFlatNormals(pMesh->vertices, pFbxMesh));
			// todo: tangents
			break;
		}
		pMesh->hasNormals = pMesh->normals.size() > 0;
		pMesh->hasTangents = pMesh->tangents.size() > 0;

		if (pMesh->hasNormals)
		{
			assert("Normal count must match vertex count!" && pMesh->normals.size() == pMesh->vertices.size());
		}
		if (pMesh->hasTangents)
		{
			assert("Tangent count must match vertex count!" && pMesh->tangents.size() == pMesh->vertices.size());
		}

		pMesh->texcoords = std::move(LoadUVInformation(pFbxMesh));

		return std::move(pMesh);
	}

	// From AutoDesk SDK examples
	static std::vector<std::vector<DirectX::XMFLOAT2>> LoadUVInformation(FbxMesh* pFbxMesh)
	{
		std::vector<std::vector<DirectX::XMFLOAT2>> result;

		FbxStringList uvSetNameList;
		pFbxMesh->GetUVSetNames(uvSetNameList);

		for (int uvIdx = 0; uvIdx < uvSetNameList.GetCount(); uvIdx++)
		{
			std::vector<DirectX::XMFLOAT2> uv(pFbxMesh->GetControlPointsCount(), DirectX::XMFLOAT2(0, 0));

			const FbxGeometryElementUV* uvSetElement = pFbxMesh->GetElementUV(uvSetNameList.GetStringAt(uvIdx));

			if (!uvSetElement)
				continue;

			if (uvSetElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
				uvSetElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
				return result;

			// Index array, where holds the index referenced to the uv data
			const bool useIndex = uvSetElement->GetReferenceMode() != FbxGeometryElement::eDirect;
			const int indexCount = (useIndex) ? uvSetElement->GetIndexArray().GetCount() : 0;
			const int polyCount = pFbxMesh->GetPolygonCount();

			if (uvSetElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				// Iterate verts
				for (int i = 0; i < pFbxMesh->GetControlPointsCount(); ++i)
				{
					int normalIndex = 0;

					// If reference mode is direct, the normal index is same as vertex index.
					if (uvSetElement->GetReferenceMode() == FbxGeometryElement::eDirect)
					{
						normalIndex = i;
					}

					// Reference mode is index-to-direct, get normals by the index-to-direct
					if (uvSetElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					{
						normalIndex = uvSetElement->GetIndexArray().GetAt(i);
					}

					// Got normals of each vertex.
					FbxVector2 uvValue = uvSetElement->GetDirectArray().GetAt(normalIndex);
					uv[i] = Vec2ToFloat2(uvValue);

				}
			}
			else if (uvSetElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				int polyIndexCounter = 0;
				for (int p = 0; p < polyCount; ++p)
				{
					// build the max index array that we need to pass into MakePoly
					const int polySize = pFbxMesh->GetPolygonSize(p);
					for (int i = 0; i < polySize; ++i)
					{
						if (polyIndexCounter < indexCount)
						{
							//the UV index depends on the reference mode
							int lUVIndex = useIndex ? uvSetElement->GetIndexArray().GetAt(polyIndexCounter) : polyIndexCounter;

							FbxVector2 uvValue = uvSetElement->GetDirectArray().GetAt(lUVIndex);

							int vertexIndex = pFbxMesh->GetPolygonVertex(p, i);
							uv[vertexIndex] = Vec2ToFloat2(uvValue);

							polyIndexCounter++;
						}
					}
				}
			}

			result.push_back(std::move(uv));
		}
		return std::move(result);
	}

	static std::vector<int> GetFbxIndices(FbxMesh* pFbxMesh)
	{
		std::vector<int> indices;
		for (int p = 0; p < pFbxMesh->GetPolygonCount(); ++p)
		{
			for (int v = 0; v < pFbxMesh->GetPolygonSize(p); ++v)
			{
				indices.push_back(pFbxMesh->GetPolygonVertex(p, v));
			}
		}
		return std::move(indices);
	}

	static std::vector<dx::XMFLOAT3> GetFbxVertices(FbxMesh* pFbxMesh)
	{
		std::vector<DirectX::XMFLOAT3> vertices;
		FbxVector4* pFbxVertices = pFbxMesh->GetControlPoints();

		for (int j = 0; j < pFbxMesh->GetControlPointsCount(); ++j)
		{
			dx::XMFLOAT3 positionOS = Vec4ToFloat3(pFbxVertices[j]);
			vertices.push_back(positionOS);
		}

		return std::move(vertices);
	}

	// https://stackoverflow.com/questions/50926809/fetching-indices-with-fbx-sdk
	/*static FbxVector4 getNormal(FbxGeometryElementNormal* normalElement, int polyIndex, int posIndex) {
		if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
			if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				return normalElement->GetDirectArray().GetAt(posIndex);
			int i = normalElement->GetIndexArray().GetAt(posIndex);
			return normalElement->GetDirectArray().GetAt(i);
		}
		else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
			if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				return normalElement->GetDirectArray().GetAt(polyIndex);
			int i = normalElement->GetIndexArray().GetAt(polyIndex);
			return normalElement->GetDirectArray().GetAt(i);
		}
		return FbxVector4();
	}*/

	static FbxVector2 getUV(FbxGeometryElementUV* uvElement, int polyIndex, int posIndex) {
		if (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
			if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				return uvElement->GetDirectArray().GetAt(posIndex);
			int i = uvElement->GetIndexArray().GetAt(posIndex);
			return uvElement->GetDirectArray().GetAt(i);
		}
		else if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
			if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				return uvElement->GetDirectArray().GetAt(polyIndex);
			int i = uvElement->GetIndexArray().GetAt(polyIndex);
			return uvElement->GetDirectArray().GetAt(i);
		}
		return FbxVector2();
	}

	//get mesh normals info
	static std::vector<DirectX::XMFLOAT3> GetFbxNormals(FbxMesh* pFbxMesh)
	{
		FbxGeometryElementNormal* normalElement = pFbxMesh->GetElementNormal();
		std::vector<DirectX::XMFLOAT3> normals(pFbxMesh->GetControlPointsCount(), DirectX::XMFLOAT3(0, 0, 0));
		if (normalElement)
		{
			if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				// If mapping mode is by control points, the mesh should be smooth and soft.

				// Iterate verts
				for (int i = 0; i < pFbxMesh->GetControlPointsCount(); ++i)
				{
					int normalIndex = 0;

					// If reference mode is direct, the normal index is same as vertex index.
					if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
					{
						normalIndex = i;
					}

					// Reference mode is index-to-direct, get normals by the index-to-direct
					if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					{
						normalIndex = normalElement->GetIndexArray().GetAt(i);
					}

					// Got normals of each vertex.
					FbxVector4 lNormal = normalElement->GetDirectArray().GetAt(normalIndex);
					//normals.push_back(Vec4ToFloat3(lNormal));
					normals[i] = Vec4ToFloat3(lNormal);

				}
			}
			else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				// Mapping mode is by polygon - vertex.

				int indexByPolygonVertex = 0;
				for (int p = 0; p < pFbxMesh->GetPolygonCount(); ++p)
				{
					// Get number of vertices in current polygon
					int lPolygonSize = pFbxMesh->GetPolygonSize(p);
					for (int i = 0; i < lPolygonSize; ++i)
					{
						int positionIndex = pFbxMesh->GetPolygonVertexIndex(p) + i;
						int normalIndex;
						if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
						{
							normalIndex = positionIndex;
						}

						if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							normalIndex = normalElement->GetIndexArray().GetAt(positionIndex);
						}

						int vertexIndex = pFbxMesh->GetPolygonVertex(p, i);

						// note: When testing, normals for same indices were the same
						// so for now, just overwriting normals by vert index
						FbxVector4 lNormal = normalElement->GetDirectArray().GetAt(normalIndex);
						normals[vertexIndex] = Vec4ToFloat3(lNormal);
					}
				}
			}
			else
			{
				throw std::runtime_error("Mesh " + std::string(pFbxMesh->GetName()) + " has unrecognized normal mapping mode");
			}
		}
		else
		{
			throw std::runtime_error("Mesh " + std::string(pFbxMesh->GetName()) + " does not have FbxGeometryElementNormal");
		}

		return std::move(normals);
	}

	//get mesh normals info
	static std::vector<DirectX::XMFLOAT3> GetFbxTangents(FbxMesh* pFbxMesh)
	{
		FbxGeometryElementTangent* tangentElement = pFbxMesh->GetElementTangent();
		std::vector<DirectX::XMFLOAT3> tangents(pFbxMesh->GetControlPointsCount(), DirectX::XMFLOAT3(0, 0, 0));
		if (tangentElement)
		{
			if (tangentElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				// If mapping mode is by control points, the mesh should be smooth and soft.

				// Iterate verts
				for (int i = 0; i < pFbxMesh->GetControlPointsCount(); ++i)
				{
					int normalIndex = 0;

					// If reference mode is direct, the normal index is same as vertex index.
					if (tangentElement->GetReferenceMode() == FbxGeometryElement::eDirect)
					{
						normalIndex = i;
					}

					// Reference mode is index-to-direct, get normals by the index-to-direct
					if (tangentElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					{
						normalIndex = tangentElement->GetIndexArray().GetAt(i);
					}

					// Got normals of each vertex.
					FbxVector4 tangentVec4 = tangentElement->GetDirectArray().GetAt(normalIndex);
					tangents[i] = Vec4ToFloat3(tangentVec4);

				}
			}
			else if (tangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				// Mapping mode is by polygon - vertex.

				int indexByPolygonVertex = 0;
				for (int p = 0; p < pFbxMesh->GetPolygonCount(); ++p)
				{
					// Get number of vertices in current polygon
					int lPolygonSize = pFbxMesh->GetPolygonSize(p);
					for (int i = 0; i < lPolygonSize; ++i)
					{
						int positionIndex = pFbxMesh->GetPolygonVertexIndex(p) + i;
						int normalIndex;
						if (tangentElement->GetReferenceMode() == FbxGeometryElement::eDirect)
						{
							normalIndex = positionIndex;
						}

						if (tangentElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							normalIndex = tangentElement->GetIndexArray().GetAt(positionIndex);
						}

						int vertexIndex = pFbxMesh->GetPolygonVertex(p, i);

						// note: When testing, normals for same indices were the same
						// so for now, just overwriting normals by vert index
						FbxVector4 tangentVec4 = tangentElement->GetDirectArray().GetAt(normalIndex);
						tangents[vertexIndex] = Vec4ToFloat3(tangentVec4);
					}
				}
			}
			else
			{
				throw std::runtime_error("Mesh " + std::string(pFbxMesh->GetName()) + " has unrecognized tangent mapping mode");
			}
		}
		else
		{
			throw std::runtime_error("Mesh " + std::string(pFbxMesh->GetName()) + " does not have FbxGeometryElementTangent");
		}

		return std::move(tangents);
	}

	static DirectX::XMFLOAT3 Vec4ToFloat3(FbxVector4 vec)
	{
		return DirectX::XMFLOAT3(
			(float)(vec.mData[0]),
			(float)(vec.mData[1]),
			(float)(vec.mData[2]));
	}

	static DirectX::XMFLOAT2 Vec2ToFloat2(FbxVector2 vec)
	{
		return DirectX::XMFLOAT2(
			(float)(vec.mData[0]),
			(float)(vec.mData[1]));
	}

	// asserts face-independent vertices w/ normals cleared to zero
	static std::vector<DirectX::XMFLOAT3> GetFlatNormals(const std::vector<DirectX::XMFLOAT3>& vertices, const FbxMesh* pMesh)
	{
		using namespace DirectX;

		std::vector<DirectX::XMFLOAT3> normals(vertices.size());

		for (int pIdx = 0; pIdx < pMesh->GetPolygonCount(); pIdx++)
		{
			int lPolygonSize = pMesh->GetPolygonSize(pIdx);
			int i0 = pMesh->GetPolygonVertex(pIdx, 0);
			int i1 = pMesh->GetPolygonVertex(pIdx, 1);
			int i2 = pMesh->GetPolygonVertex(pIdx, 2);

			// For now, just take first 3 verts
			auto& v0 = vertices[i0];
			auto& v1 = vertices[i1];
			auto& v2 = vertices[i2];

			// Convert to vectors
			const auto p0 = XMLoadFloat3(&v0);
			const auto p1 = XMLoadFloat3(&v1);
			const auto p2 = XMLoadFloat3(&v2);

			const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

			XMStoreFloat3(&normals[i0], n);
			XMStoreFloat3(&normals[i1], n);
			XMStoreFloat3(&normals[i2], n);
		}

		return normals;
	}

};