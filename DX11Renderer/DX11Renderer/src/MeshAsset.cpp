#include "pch.h"
#include "MeshAsset.h"

namespace gfx
{
    const u32 MeshAsset::GetVertexCount() const
    {
        return (u32)m_vertices.size();
    }

    const u32 MeshAsset::GetIndexCount() const
    {
        return (u32)m_indices.size();
    }

    const u32 MeshAsset::GetTexcoordCount() const
    {
        return (u32)m_texcoords.size();
    }

    const AABB& MeshAsset::GetAABB() const
    {
        return m_aabb;
    }

    const dx::XMFLOAT3 MeshAsset::GetVertexPosition(const size_t vertexIdx) const
    {
        return m_vertices.at(vertexIdx);
    }

    const u32 MeshAsset::GetIndex(const size_t indexIdx) const
    {
        return m_indices.at(indexIdx);
    }

    const dx::XMFLOAT3 MeshAsset::GetNormalOrDefault(const size_t vertexIdx) const
    {
        return (hasNormals) ? m_normals.at(vertexIdx) : dx::XMFLOAT3(0, 0, 1);
    }

    const dx::XMFLOAT4 MeshAsset::GetTangentOrDefault(const size_t vertexIdx) const
    {
        return (hasTangents) ? m_tangents.at(vertexIdx) : dx::XMFLOAT4(0, 0, 1, 0);
    }

    const dx::XMFLOAT2 MeshAsset::GetTexcoordOrDefault(const size_t vertexIdx, const size_t texcoordIdx) const
    {
        return (texcoordIdx < m_texcoords.size() && vertexIdx < m_texcoords.at(texcoordIdx).size()) ? m_texcoords.at(texcoordIdx).at(vertexIdx) : dx::XMFLOAT2(0, 0);
    }
}