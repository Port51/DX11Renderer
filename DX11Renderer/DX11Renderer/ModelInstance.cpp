#include "ModelInstance.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "PixelConstantBuffer.h"
#include "FBXLoader.h"
#include "VertexInclude.h"
//#include "Sphere.h"

namespace dx = DirectX;

ModelInstance::ModelInstance(Graphics& gfx, const std::string filename, DirectX::XMFLOAT3 materialColor, dx::XMFLOAT3 instanceScale)
{
	const auto pScene = imp.ReadFile(filename.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices
	);

	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
	}

	pSceneGraph = ParseNode(*pScene->mRootNode);
}

void ModelInstance::Draw(Graphics& gfx, DirectX::FXMMATRIX transform) const
{
	pSceneGraph->Draw(gfx, transform);
}