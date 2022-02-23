#pragma once
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "DXMathInclude.h"
#include "BaseMesh.h"

namespace gfx
{
	///
	/// Storage class that can be used to make mesh instances
	///
	class MeshAsset : public BaseMesh
	{
	public:
		MeshAsset() = default;
		virtual ~MeshAsset() = default;

	public:
		// Store vertex attributes separately, and combine when making instances based on what attributes are needed
		std::vector<dx::XMFLOAT3> vertices;

		bool hasNormals;
		std::vector<dx::XMFLOAT3> normals;

		bool hasTangents;
		std::vector<dx::XMFLOAT3> tangents;

		std::vector<std::vector<dx::XMFLOAT2>> texcoords;

		UINT numFaces;
		std::vector<u32> indices;

		int materialIndex;
	};
}