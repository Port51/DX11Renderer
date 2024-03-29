#pragma once
#include "BaseMesh.h"
#include <vector>
#include "DXMathInclude.h"
#include "AABB.h"

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
		std::vector<dx::XMFLOAT3> m_vertices;

		bool hasNormals;
		std::vector<dx::XMFLOAT3> m_normals;

		bool hasTangents;
		std::vector<dx::XMFLOAT4> m_tangents;

		bool hasVertColors;
		std::vector<dx::XMFLOAT4> m_vertColors;

		std::vector<std::vector<dx::XMFLOAT2>> m_texcoords;

		UINT m_numFaces;
		std::vector<u32> m_indices;

		UINT m_faceDimension;

		int m_materialIndex;

		AABB m_aabb;
	};
}