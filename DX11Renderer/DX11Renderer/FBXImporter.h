#pragma once
#include <assert.h>
#include <stdio.h>
#include <fbxsdk.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "SceneGraphNode.h"

namespace dx = DirectX;

#define FBX_IMPORTER_VERBOSE

// Memory manager object
static FbxManager* fbxSdkManager = nullptr;

//template<class V> // template on vector type
class FBXImporter
{
public:
	//template<class V>
	static std::unique_ptr<ModelAsset> LoadFBX(const char* filename, bool verbose = false)
	{
		// Refs:
		// * http://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_getting_started_your_first_fbx_sdk_program_html

		std::unique_ptr<ModelAsset> pOutputModel;

		// Init memory manager if needed
		if (fbxSdkManager == nullptr)
		{
#if defined(FBX_IMPORTER_VERBOSE)
			printf("FBX Importer: Initializing FBX SDK manager...\n");
#endif
			fbxSdkManager = FbxManager::Create();

			FbxIOSettings* pIOsettings = FbxIOSettings::Create(fbxSdkManager, IOSROOT);
			fbxSdkManager->SetIOSettings(pIOsettings);
		}

#if defined(FBX_IMPORTER_VERBOSE)
		printf("FBX Importer: Creating FBX importer...\n");
#endif
		FbxImporter* pImporter = FbxImporter::Create(fbxSdkManager, "");
		if (pImporter->Initialize(filename, -1, fbxSdkManager->GetIOSettings()))
		{
#if defined(FBX_IMPORTER_VERBOSE)
			printf("FBX Importer: Importing FBX scene...\n");
#endif
			FbxScene* pFbxScene = FbxScene::Create(fbxSdkManager, "");
			if (pImporter->Import(pFbxScene))
			{
				pImporter->Destroy(); // no need to keep this around
#if defined(FBX_IMPORTER_VERBOSE)
				printf("FBX Importer: Processing scene graph...\n");
#endif

				// The root node should not contain any attributes
				//auto pFbxRootNode = std::make_unique<FbxNode>(pFbxScene->GetRootNode());
				FbxNode* pFbxRootNode = pFbxScene->GetRootNode();
				if (pFbxRootNode)
				{
					//auto pSceneGraphRootNode = UnpackFbxSceneGraph(std::move(pFbxRootNode));
#if defined(FBX_IMPORTER_VERBOSE)
					printf("FBX Importer: Creating model scene graph...\n");
#endif
					auto pSceneGraph = UnpackFbxSceneGraph(pFbxRootNode);
#if defined(FBX_IMPORTER_VERBOSE)
					printf("FBX Importer: Creating model asset...\n");
#endif
					auto pModelAsset = std::make_unique<ModelAsset>(std::move(pSceneGraph));

					return std::move(pModelAsset);
				}
			}
		}

		pImporter->Destroy();
		return nullptr;
	}

	// Recursively unpack scene graph and return root node
	//static std::unique_ptr<SceneGraphNode<MeshAsset>> UnpackFbxSceneGraph(std::unique_ptr<FbxNode> pFbxNode)
	static std::unique_ptr<SceneGraphNode<MeshAsset>> UnpackFbxSceneGraph(FbxNode* pFbxNode)
	{
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

#if defined(FBX_IMPORTER_VERBOSE)
				printf("FBX Importer: Import mesh ");
				printf(pFbxNode->GetName());
				printf("...\n");
#endif
				pMeshAsset = LoadFbxMesh(std::string(pFbxNode->GetName()), pFbxMesh);
			}
		}

		// Iterate through children
		auto pChildNodes = std::vector<std::unique_ptr<SceneGraphNode<MeshAsset>>>();
		for (int i = 0; i < pFbxNode->GetChildCount(); i++)
		{
			//auto pFbxChildNode = std::make_unique<FbxNode>(pFbxNode->GetChild(i));
			FbxNode* pFbxChildNode = pFbxNode->GetChild(i);
			if (pFbxChildNode->GetNodeAttribute())
			{
				//pChildNodes.emplace_back(UnpackFbxSceneGraph(std::move(pFbxChildNode)));
				pChildNodes.emplace_back(UnpackFbxSceneGraph(pFbxChildNode));
			}
		}

		auto pResult
			= std::make_unique<SceneGraphNode<MeshAsset>>(std::move(pMeshAsset), std::move(pChildNodes));
		return std::move(pResult);
	}

	//static std::unique_ptr<MeshAsset> LoadFbxMesh(std::unique_ptr<FbxMesh> pFbxMesh)
	static std::unique_ptr<MeshAsset> LoadFbxMesh(std::string name, FbxMesh* pFbxMesh)
	{
		auto pMesh = std::make_unique<MeshAsset>();
		pMesh->name = name;

		FbxVector4* pFbxVertices = pFbxMesh->GetControlPoints();

		// To convert to XMFLOAT3, use this:
		// *reinterpret_cast<dx::XMFLOAT3*>(&pMesh->mNormals[i])

		// Vertices
		for (int j = 0; j < pFbxMesh->GetControlPointsCount(); ++j)
		{
			//auto fbxVertInfo = pFbxVertices.get()[j];
			auto fbxVertInfo = pFbxVertices[j];
			dx::XMFLOAT3 positionOS = Vec4ToFloat3(fbxVertInfo);
			
			pMesh->vertices.push_back(positionOS);
		}

		int* pIndices = pFbxMesh->GetPolygonVertices();

		// Indices
		for (int j = 0; j < pFbxMesh->GetPolygonVertexCount(); ++j)
		{
			pMesh->indices.push_back(pIndices[j]);
		}

		//pMesh->normals = std::move(GetNormals(pFbxMesh));
		pMesh->normals = GetFlatNormals(pMesh->vertices, pMesh->indices);
		pMesh->hasNormals = pMesh->normals.size() > 0;

		return std::move(pMesh);
	}

	// From AutoDesk SDK examples
	void LoadUVInformation(FbxMesh* pMesh)
	{
		//get all UV set names
		FbxStringList lUVSetNameList;
		pMesh->GetUVSetNames(lUVSetNameList);

		//iterating over all uv sets
		for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
		{
			//get lUVSetIndex-th uv set
			const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
			const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

			if (!lUVElement)
				continue;

			// only support mapping mode eByPolygonVertex and eByControlPoint
			if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
				lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
				return;

			//index array, where holds the index referenced to the uv data
			const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
			const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

			//iterating through the data by polygon
			const int lPolyCount = pMesh->GetPolygonCount();

			if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
				{
					// build the max index array that we need to pass into MakePoly
					const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
					for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
					{
						FbxVector2 lUVValue;

						//get the index of the current vertex in control points array
						int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

						//User TODO:
						//Print out the value of UV(lUVValue) or log it to a file
					}
				}
			}
			else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				int lPolyIndexCounter = 0;
				for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
				{
					// build the max index array that we need to pass into MakePoly
					const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
					for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
					{
						if (lPolyIndexCounter < lIndexCount)
						{
							FbxVector2 lUVValue;

							//the UV index depends on the reference mode
							int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

							lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

							//User TODO:
							//Print out the value of UV(lUVValue) or log it to a file

							lPolyIndexCounter++;
						}
					}
				}
			}
		}
	}

	//get mesh normals info
	static std::vector<DirectX::XMFLOAT3> GetNormals(FbxMesh* pMesh)
	{
		std::vector<DirectX::XMFLOAT3> normals;

		//get the normal element
		FbxGeometryElementNormal* lNormalElement = pMesh->GetElementNormal();
		if (lNormalElement)
		{
			//mapping mode is by control points. The mesh should be smooth and soft.
			//we can get normals by retrieving each control point
			if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				//Let's get normals of each vertex, since the mapping mode of normal element is by control point
				for (int lVertexIndex = 0; lVertexIndex < pMesh->GetControlPointsCount(); lVertexIndex++)
				{
					int lNormalIndex = 0;
					//reference mode is direct, the normal index is same as vertex index.
					//get normals by the index of control vertex
					if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
						lNormalIndex = lVertexIndex;

					//reference mode is index-to-direct, get normals by the index-to-direct
					if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						lNormalIndex = lNormalElement->GetIndexArray().GetAt(lVertexIndex);

					//Got normals of each vertex.
					FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
					normals.push_back(Vec4ToFloat3(lNormal));

				}//end for lVertexIndex
			}//end eByControlPoint
			//mapping mode is by polygon-vertex.
			//we can get normals by retrieving polygon-vertex.
			else if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				int lIndexByPolygonVertex = 0;
				//Let's get normals of each polygon, since the mapping mode of normal element is by polygon-vertex.
				for (int lPolygonIndex = 0; lPolygonIndex < pMesh->GetPolygonCount(); lPolygonIndex++)
				{
					//get polygon size, you know how many vertices in current polygon.
					int lPolygonSize = pMesh->GetPolygonSize(lPolygonIndex);
					//retrieve each vertex of current polygon.
					for (int i = 0; i < lPolygonSize; i++)
					{
						int lNormalIndex = 0;
						//reference mode is direct, the normal index is same as lIndexByPolygonVertex.
						if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
							lNormalIndex = lIndexByPolygonVertex;

						//reference mode is index-to-direct, get normals by the index-to-direct
						if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
							lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndexByPolygonVertex);

						//Got normals of each polygon-vertex.
						FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
						normals.push_back(Vec4ToFloat3(lNormal));

						lIndexByPolygonVertex++;
					}//end for i //lPolygonSize
				}//end for lPolygonIndex //PolygonCount

			}//end eByPolygonVertex
		}//end if lNormalElement

		return std::move(normals);
	}

	static DirectX::XMFLOAT3 Vec4ToFloat3(FbxVector4 vec)
	{
		return DirectX::XMFLOAT3(
			static_cast<float>(vec[0]),
			static_cast<float>(vec[1]),
			static_cast<float>(vec[2]));
	}

	// asserts face-independent vertices w/ normals cleared to zero
	static std::vector<DirectX::XMFLOAT3> GetFlatNormals(const std::vector<DirectX::XMFLOAT3>& vertices, const std::vector<int>& indices)
	{
		using namespace DirectX;

		std::vector<DirectX::XMFLOAT3> normals (vertices.size());
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			auto& v0 = vertices[indices[i]];
			auto& v1 = vertices[indices[i + 1]];
			auto& v2 = vertices[indices[i + 2]];

			// Convert to vectors
			const auto p0 = XMLoadFloat3(&v0);
			const auto p1 = XMLoadFloat3(&v1);
			const auto p2 = XMLoadFloat3(&v2);

			const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

			XMStoreFloat3(&normals[indices[i]], n);
			XMStoreFloat3(&normals[indices[i + 1]], n);
			XMStoreFloat3(&normals[indices[i + 2]], n);
		}

		return normals;
	}

};