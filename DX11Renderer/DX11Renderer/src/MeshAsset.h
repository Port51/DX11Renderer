#pragma once
#include "CommonHeader.h"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "DXMathInclude.h"
#include "AABB.h"
#include "BaseMesh.h"

namespace gfx
{
	///
	/// Storage class that can be used to make mesh instances
	///
	class MeshAsset : public BaseMesh
	{
		friend class ModelImporter; // allow ModelImporter to setup this class
	public:
		MeshAsset() = default;
		virtual ~MeshAsset() = default;

		const u32 GetVertexCount() const;
		const u32 GetIndexCount() const;
		const u32 GetTexcoordCount() const;

		const AABB& GetAABB() const;
		const dx::XMFLOAT3 GetVertexPosition(const size_t vertexIdx) const;
		const u32 GetIndex(const size_t indexIdx) const;
		const dx::XMFLOAT3 GetNormalOrDefault(const size_t vertexIdx) const;
		const dx::XMFLOAT4 GetTangentOrDefault(const size_t vertexIdx) const;
		const dx::XMFLOAT2 GetTexcoordOrDefault(const size_t vertexIdx, const size_t texcoordIdx) const;

	public: // todo: make private!
		// Store vertex attributes separately, and combine when making instances based on what attributes are needed
		std::vector<dx::XMFLOAT3> m_vertices;

		bool hasNormals;
		std::vector<dx::XMFLOAT3> m_normals;

		bool hasTangents;
		std::vector<dx::XMFLOAT4> m_tangents;

		std::vector<std::vector<dx::XMFLOAT2>> m_texcoords;

		UINT m_numFaces;
		std::vector<u32> m_indices;

		int m_materialIndex;

		AABB m_aabb;
	};
}