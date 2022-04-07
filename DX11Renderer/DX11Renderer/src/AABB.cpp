#include "pch.h"
#include "AABB.h"
#include "MeshRenderer.h"
#include "SceneGraphNode.h"
#include "Frustum.h"

namespace gfx
{
	AABB::AABB()
	{
		Clear();
	}

	void AABB::Clear()
	{
		m_centerLS = dx::XMFLOAT3(0.f, 0.f, 0.f);
		m_extentsLS = dx::XMFLOAT3(0.f, 0.f, 0.f);
	}

	void AABB::SetBoundsByVertices(const std::vector<dx::XMFLOAT3>& vertices)
	{
		dx::XMVECTOR minCornerOS = dx::g_XMInfinity;
		dx::XMVECTOR maxCornerOS = dx::g_XMNegInfinity;

		for (int vi = 0; vi < vertices.size(); ++vi)
		{
			const auto posV3 = dx::XMLoadFloat3(&vertices[vi]);
			minCornerOS = dx::XMVectorMin(minCornerOS, posV3);
			maxCornerOS = dx::XMVectorMax(maxCornerOS, posV3);
		}

		SetBoundsByMinMaxCorners(minCornerOS, maxCornerOS);
	}

	void AABB::ExpandBoundsToFitAABB(const AABB& other)
	{
		// Assume there is no local offset
		SetBoundsByMinMaxCorners(
			dx::XMVectorMin(GetMinimumCornerLS(), other.GetMinimumCornerLS()),
			dx::XMVectorMax(GetMaximumCornerLS(), other.GetMaximumCornerLS())
		);
	}

	void AABB::ExpandBoundsToFitAABB(const AABB& other, const dx::XMVECTOR offset)
	{
		// Use offset to transform the other AABB into local space of current AABB
		SetBoundsByMinMaxCorners(
			dx::XMVectorMin(GetMinimumCornerLS(), dx::XMVectorAdd(other.GetMinimumCornerLS(), offset)),
			dx::XMVectorMax(GetMaximumCornerLS(), dx::XMVectorAdd(other.GetMaximumCornerLS(), offset))
		);
	}

	void AABB::ExpandBoundsToFitChildNodes(const std::vector<std::shared_ptr<SceneGraphNode>>& pChildNodes)
	{
		// It's faster to determine min/max corners from all AABBs first
		auto minCornerWS = GetMinimumCornerLS();
		auto maxCornerWS = GetMaximumCornerLS();
		for (int i = 0, ct = pChildNodes.size(); i < ct; ++i)
		{
			// Use offset to transform the other AABB into local space of current AABB
			const auto localTransformOffset = dx::XMLoadFloat3(&pChildNodes[i]->m_localTransformOffset);
			minCornerWS = dx::XMVectorMin(minCornerWS, dx::XMVectorAdd(pChildNodes[i]->m_boundingVolumeAABB.GetMinimumCornerLS(), localTransformOffset));
			maxCornerWS = dx::XMVectorMin(maxCornerWS, dx::XMVectorAdd(pChildNodes[i]->m_boundingVolumeAABB.GetMaximumCornerLS(), localTransformOffset));
		}

		SetBoundsByMinMaxCorners(minCornerWS, maxCornerWS);
	}

	void AABB::SetBoundsByMinMaxCorners(const dx::XMVECTOR minimumCornerWS, const dx::XMVECTOR maximumCornerWS)
	{
		// CenterWS  = (max + min) * 0.5
		// ExtentsWS = (max - min) * 0.5
		dx::XMStoreFloat3(&m_centerLS, dx::XMVectorScale(dx::XMVectorAdd(maximumCornerWS, minimumCornerWS), 0.5f));
		dx::XMStoreFloat3(&m_extentsLS, dx::XMVectorScale(dx::XMVectorSubtract(maximumCornerWS, minimumCornerWS), 0.5f));
	}

	const dx::XMVECTOR AABB::GetCenterWS(const dx::XMVECTOR transformWS) const
	{
		return dx::XMVectorAdd(transformWS, GetCenterLS());
	}

	const dx::XMVECTOR AABB::GetExtentsWS(const dx::XMVECTOR transformWS) const
	{
		// Assume AABB measurements have scale baked in already
		return GetExtentsLS();
	}

	const dx::XMVECTOR AABB::GetMinimumCornerWS(const dx::XMVECTOR transformWS) const
	{
		return dx::XMVectorAdd(transformWS, GetMinimumCornerLS());
	}

	const dx::XMVECTOR AABB::GetMaximumCornerWS(const dx::XMVECTOR transformWS) const
	{
		return dx::XMVectorAdd(transformWS, GetMaximumCornerLS());
	}

	const dx::XMVECTOR AABB::GetCenterLS() const
	{
		return dx::XMLoadFloat3(&m_centerLS);
	}

	const dx::XMVECTOR AABB::GetExtentsLS() const
	{
		return dx::XMLoadFloat3(&m_extentsLS);
	}

	const dx::XMVECTOR AABB::GetMinimumCornerLS() const
	{
		return dx::XMVectorSubtract(GetCenterLS(), GetExtentsLS());
	}

	const dx::XMVECTOR AABB::GetMaximumCornerLS() const
	{
		return dx::XMVectorAdd(GetCenterLS(), GetExtentsLS());
	}

	const bool AABB::DoesPointIntersect(const dx::XMVECTOR positionWS) const
	{
		// Displacement must be <= extents
		const auto absDisplWS = dx::XMVectorAbs(dx::XMVectorSubtract(positionWS, GetCenterLS()));
		return dx::XMVector3LessOrEqual(absDisplWS, GetExtentsLS());
	}

	const bool AABB::DoesSphereIntersect(const dx::XMVECTOR positionAndRadius, const dx::XMVECTOR aabbObjectPosition) const
	{
		float range = dx::XMVectorGetW(positionAndRadius);

		// Formula: float3 displ = max(0, abs(aabbCenter - spherePos) - aabbExtents);
		dx::XMVECTOR displ = dx::XMVectorAbs(dx::XMVectorSubtract(dx::XMVectorAdd(dx::XMLoadFloat3(&m_centerLS), aabbObjectPosition), dx::XMVectorSetW(positionAndRadius, 0.f)));
		displ = dx::XMVectorSubtract(displ, dx::XMLoadFloat3(&m_extentsLS));
		displ = dx::XMVectorMax(dx::XMVectorZero(), displ);

		float sdfSqr;
		dx::XMStoreFloat(&sdfSqr, dx::XMVector3Dot(displ, displ));

		return sdfSqr <= range * range;
	}

	const bool AABB::DoesAABBIntersect(const AABB& other) const
	{
		// Displacement must be <= combined extents
		const auto absDisplWS = dx::XMVectorAbs(dx::XMVectorSubtract(other.GetCenterLS(), GetCenterLS()));
		return dx::XMVector3LessOrEqual(absDisplWS, dx::XMVectorAdd(other.GetExtentsLS(), GetExtentsLS()));
	}

	/*const bool AABB::FastFrustumIntersects(const Frustum& frustum) const
	{
		// Reference: https://www.gamedev.net/forums/topic/512123-fast--and-correct-frustum---aabb-intersection/
		const auto minCornerWS = GetMinimumCornerLS();
		const auto maxCornerWS = GetMaximumCornerLS();

		dx::XMVECTOR vmin = dx::XMVectorZero();
		for (int i = 0; i < 6; ++i) {
			// X axis 
			if (frustum.planes[i].x > 0) {
				dx::XMVectorSetX(vmin, dx::XMVectorGetX(minCornerWS));
			}
			else {
				dx::XMVectorSetX(vmin, dx::XMVectorGetX(maxCornerWS));
			}

			// Y axis 
			if (frustum.planes[i].y > 0) {
				dx::XMVectorSetY(vmin, dx::XMVectorGetY(minCornerWS));
			}
			else {
				dx::XMVectorSetY(vmin, dx::XMVectorGetY(maxCornerWS));
			}

			// Z axis 
			if (frustum.planes[i].z > 0) {
				dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(minCornerWS));
			}
			else {
				dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(maxCornerWS));
			}

			if (dx::XMVectorGetX(dx::XMVector3Dot(dx::XMLoadFloat4(&frustum.planes[i]), vmin)) + frustum.planes[i].w > 0.f)
			{
				return false;
			}
		}
		return true;
	}

	// Return values: 0 = intersection, 1 = inside frustum, 2 = outside frustum
	const int AABB::FrustumIntersects(const Frustum& frustum) const
	{
		// Reference: https://www.gamedev.net/forums/topic/512123-fast--and-correct-frustum---aabb-intersection/
		int result = 1;
		const auto minCornerWS = GetMinimumCornerLS();
		const auto maxCornerWS = GetMaximumCornerLS();

		dx::XMVECTOR vmin = dx::XMVectorZero();
		dx::XMVECTOR vmax = dx::XMVectorZero();
		for (int i = 0; i < 6; ++i) {
			// X axis 
			if (frustum.planes[i].x > 0) {
				dx::XMVectorSetX(vmin, dx::XMVectorGetX(minCornerWS));
				dx::XMVectorSetX(vmax, dx::XMVectorGetX(maxCornerWS));
			}
			else {
				dx::XMVectorSetX(vmin, dx::XMVectorGetX(maxCornerWS));
				dx::XMVectorSetX(vmax, dx::XMVectorGetX(minCornerWS));
			}

			// Y axis 
			if (frustum.planes[i].y > 0) {
				dx::XMVectorSetY(vmin, dx::XMVectorGetY(minCornerWS));
				dx::XMVectorSetY(vmax, dx::XMVectorGetY(maxCornerWS));
			}
			else {
				dx::XMVectorSetY(vmin, dx::XMVectorGetY(maxCornerWS));
				dx::XMVectorSetY(vmax, dx::XMVectorGetY(minCornerWS));
			}

			// Z axis 
			if (frustum.planes[i].z > 0) {
				dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(minCornerWS));
				dx::XMVectorSetZ(vmax, dx::XMVectorGetZ(maxCornerWS));
			}
			else {
				dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(maxCornerWS));
				dx::XMVectorSetZ(vmax, dx::XMVectorGetZ(minCornerWS));
			}

			if (dx::XMVectorGetX(dx::XMVector3Dot(dx::XMLoadFloat4(&frustum.planes[i]), vmin)) + frustum.planes[i].w > 0.f)
			{
				// Outside frustum
				return 2;
			}
			if (dx::XMVectorGetX(dx::XMVector3Dot(dx::XMLoadFloat4(&frustum.planes[i]), vmax)) + frustum.planes[i].w >= 0.f)
			{
				// Intersection
				result = 0;
			}
		}
		return result;
	}*/
}