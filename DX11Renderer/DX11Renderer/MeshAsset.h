#pragma once
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include "DebugHelper.h"
#include "BaseMesh.h"

namespace dx = DirectX;

class MeshAsset : public BaseMesh
{
public:
	MeshAsset()
	{
	}

public:
	// Store vertex attributes separately, and combine them before sending to GPU
	std::vector<dx::XMFLOAT3> vertices;

	bool hasNormals;
	std::vector<dx::XMFLOAT3> normals;

	// Mirror the way Unity stores texcoords (for now)
	size_t numTexcoords;
	std::vector<dx::XMFLOAT3> texcoord0;
	std::vector<dx::XMFLOAT3> texcoord1;
	std::vector<dx::XMFLOAT3> texcoord2;
	std::vector<dx::XMFLOAT3> texcoord3;

	size_t numFaces;
	std::vector<int> indices;
};