#pragma once
#include <fbxsdk.h>
#include <vector>
#include "WindowsInclude.h"
#include "GraphicsThrowMacros.h"
#include "IndexedTriangleList.h"

static FbxManager* g_pFbxSdkManager = nullptr;

class FBXLoader
{

public:
	template<class V>
	static HRESULT LoadFBX(const char* filename, IndexedTriangleList<V>* pResult)
	{
		/*V vertex;

		vertex.pos = { -10.f, 0.f, 0.f };
		pResult->vertices.push_back(vertex);

		vertex.pos = { -10.f, 10.f, 0.f };
		pResult->vertices.push_back(vertex);

		vertex.pos = { 0.f, 0.f, 0.f };
		pResult->vertices.push_back(vertex);

		pResult->indices.push_back(0u);
		pResult->indices.push_back(1u);
		pResult->indices.push_back(2u);

		return S_OK;*/

		if (g_pFbxSdkManager == nullptr)
		{
			g_pFbxSdkManager = FbxManager::Create();

			FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
			g_pFbxSdkManager->SetIOSettings(pIOsettings);
		}

		FbxImporter* pImporter = FbxImporter::Create(g_pFbxSdkManager, "");
		FbxScene* pFbxScene = FbxScene::Create(g_pFbxSdkManager, "");
		//std::unique_ptr<FbxImporter> pImporter(FbxImporter::Create(g_pFbxSdkManager, ""));
		//std::unique_ptr<FbxScene> pFbxScene(FbxScene::Create(g_pFbxSdkManager, ""));

		bool bSuccess = pImporter->Initialize(filename, -1, g_pFbxSdkManager->GetIOSettings());
		if (!bSuccess) return E_FAIL;

		bSuccess = pImporter->Import(pFbxScene);
		if (!bSuccess) return E_FAIL;

		pImporter->Destroy();

		FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

		if (pFbxRootNode)
		{
			for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
			{
				FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

				if (pFbxChildNode->GetNodeAttribute() == NULL)
					continue;

				FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

				if (AttributeType != FbxNodeAttribute::eMesh)
					continue;

				FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();

				FbxVector4* pVertices = pMesh->GetControlPoints();
				//pMesh->GetTextureUV()

				// Verts
				for (int j = 0; j < pMesh->GetControlPointsCount(); ++j)
				{
					V vertex;
					vertex.pos = { (float)pVertices[j].mData[0], (float)pVertices[j].mData[1], (float)pVertices[j].mData[2] };
					pResult->vertices.push_back(vertex);
				}

				int* pIndices = pMesh->GetPolygonVertices();

				// Indices
				for (int j = 0; j < pMesh->GetPolygonVertexCount(); ++j)
				{
					pResult->indices.push_back(pIndices[j]);
				}

			}

		}

		//pResult->indices

		return S_OK;
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
};