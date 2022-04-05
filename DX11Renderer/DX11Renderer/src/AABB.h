#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"
#include <vector>

namespace gfx
{
	class MeshRenderer;
	class SceneGraphNode;

	struct Frustum;

	class AABB
	{
		friend struct Frustum;
	public:
		//enum AABBCoordinateSpace { WorldSpace, ObjectSpace };
	public:
		AABB();
		~AABB() = default;
	public:
		void Clear();
		void SetBoundsByVertices(const std::vector<dx::XMFLOAT3>& vertices);
		void ExpandBoundsToFitAABB(const AABB& other);
		void ExpandBoundsToFitAABB(const AABB& other, const dx::XMVECTOR offset);
		void ExpandBoundsToFitChildNodes(const std::vector<std::shared_ptr<SceneGraphNode>>& pChildNodes);
		void SetBoundsByMinMaxCorners(const dx::XMVECTOR minimumCornerWS, const dx::XMVECTOR maximumCornerWS);
		const dx::XMVECTOR GetCenterWS(const dx::XMVECTOR transformWS) const;
		const dx::XMVECTOR GetExtentsWS(const dx::XMVECTOR transformWS) const;
		const dx::XMVECTOR GetMinimumCornerWS(const dx::XMVECTOR transformWS) const;
		const dx::XMVECTOR GetMaximumCornerWS(const dx::XMVECTOR transformWS) const;
	protected:
		const dx::XMVECTOR GetCenterLS() const;
		const dx::XMVECTOR GetExtentsLS() const;
		const dx::XMVECTOR GetMinimumCornerLS() const;
		const dx::XMVECTOR GetMaximumCornerLS() const;
	public:
		const bool DoesPointIntersect(const dx::XMVECTOR positionWS) const;
		const bool DoesSphereIntersect(const dx::XMVECTOR positionAndRadius, const dx::XMVECTOR aabbObjectPosition) const;
		const bool DoesAABBIntersect(const AABB& other) const;
		// Returns true if AABB intersects or is inside frustum
		//const bool FastFrustumIntersects(const Frustum& frustum) const;
		// Returns 0 if AABB intersects, 1 if AABB is inside frustum, and 2 if AABB is outside frustum
		//const int FrustumIntersects(const Frustum& frustum) const;
	private:
		// todo: is it better to store min/max corners as well? this would speed up frustum testing a bit, but double memory.

		// Local space center
		dx::XMFLOAT3 m_centerLS;
		// Local space extents
		dx::XMFLOAT3 m_extentsLS;
	};
}