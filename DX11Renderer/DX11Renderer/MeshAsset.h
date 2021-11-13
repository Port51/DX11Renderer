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

	bool hasTangents;
	std::vector<dx::XMFLOAT3> tangents;

	std::vector<std::vector<DirectX::XMFLOAT2>> texcoords;

	size_t numFaces;
	std::vector<int> indices;

	int materialIndex;
};