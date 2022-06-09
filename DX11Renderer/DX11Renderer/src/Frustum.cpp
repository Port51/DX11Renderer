#include "pch.h"
#include "Frustum.h"
#include "AABB.h"

namespace gfx
{

	Frustum::Frustum()
	{
		planes.resize(PlaneCt);
	}

	Frustum::Frustum(std::vector<dx::XMFLOAT4> planes)
		: planes(planes)
	{
		assert(planes.size() == PlaneCt && "Wrong number of Frustum planes");
	}

	void Frustum::UpdatePlanesFromMatrix(dx::XMMATRIX inputMatrix)
	{
		// Reference: http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
		// Note that plane equations point INWARD

		// Need access to individual components
		dx::XMFLOAT4X4 matrix;
		dx::XMStoreFloat4x4(&matrix, inputMatrix);

		// Note: use dx::XMPlaneNormalize(), which is equivalent to:
		// m = sqrt(x^2+y^2+z^2)
		// [x/m, y/m, z/m, w/m]

		// Left plane
		dx::XMStoreFloat4(&planes[2],
			dx::XMPlaneNormalize(
				dx::XMVectorSet(
					matrix._14 + matrix._11,
					matrix._24 + matrix._21,
					matrix._34 + matrix._31,
					matrix._44 + matrix._41
				)));

		// Right plane
		dx::XMStoreFloat4(&planes[3],
			dx::XMPlaneNormalize(
				dx::XMVectorSet(
					matrix._14 - matrix._11,
					matrix._24 - matrix._21,
					matrix._34 - matrix._31,
					matrix._44 - matrix._41
				)));

		// Top plane
		dx::XMStoreFloat4(&planes[4],
			dx::XMPlaneNormalize(
				dx::XMVectorSet(
					matrix._14 - matrix._12,
					matrix._24 - matrix._22,
					matrix._34 - matrix._32,
					matrix._44 - matrix._42
				)));

		// Bottom plane
		dx::XMStoreFloat4(&planes[5],
			dx::XMPlaneNormalize(
				dx::XMVectorSet(
					matrix._14 + matrix._12,
					matrix._24 + matrix._22,
					matrix._34 + matrix._32,
					matrix._44 + matrix._42
				)));

		// Near plane
		dx::XMStoreFloat4(&planes[0],
			dx::XMPlaneNormalize(
				dx::XMVectorSet(
					matrix._13,
					matrix._23,
					matrix._33,
					matrix._43
				)));

		// Far plane
		dx::XMStoreFloat4(&planes[1],
			dx::XMPlaneNormalize(
				dx::XMVectorSet(
					matrix._14 - matrix._13,
					matrix._24 - matrix._23,
					matrix._34 - matrix._33,
					matrix._44 - matrix._43
				)));
	}

	void Frustum::UpdatePlanesFromViewSpaceCorners(const dx::XMVECTOR frustumCorners, const float nearPlane, const float farPlane)
	{
		// Use cross product to turn tile view directions into plane directions
		// The cross product is done by flipping X or Y with Z
		// Also, flip y here as it was previously flipped for GPU
		dx::XMStoreFloat4(&planes[2], dx::XMVector3Normalize(dx::XMVectorSet(1.f, 0.f, dx::XMVectorGetX(frustumCorners), 0.f)));
		dx::XMStoreFloat4(&planes[3], dx::XMVector3Normalize(dx::XMVectorSet(-1.f, 0.f, dx::XMVectorGetX(frustumCorners), 0.f)));
		dx::XMStoreFloat4(&planes[4], dx::XMVector3Normalize(dx::XMVectorSet(0.f, -1.f, -dx::XMVectorGetY(frustumCorners), 0.f)));
		dx::XMStoreFloat4(&planes[5], dx::XMVector3Normalize(dx::XMVectorSet(0.f, 1.f, -dx::XMVectorGetY(frustumCorners), 0.f)));

		// Near/far planes
		dx::XMStoreFloat4(&planes[0], dx::XMVectorSet(0.f, 0.f, 1.f, -nearPlane));
		dx::XMStoreFloat4(&planes[1], dx::XMVectorSet(0.f, 0.f, -1.f, farPlane));
	}

	const dx::XMVECTOR Frustum::GetGPUFrustumPlaneDir() const
	{
		const auto v0 = dx::XMLoadFloat4(&planes[2]);
		const auto v1 = dx::XMLoadFloat4(&planes[5]);
		return dx::XMVectorSet(
			dx::XMVectorGetX(v0),
			-dx::XMVectorGetZ(v0),
			dx::XMVectorGetY(v1),
			-dx::XMVectorGetZ(v1));
	}

	// Fast version that only checks for intersection
	const bool Frustum::DoesAABBIntersect(const AABB& aabb, const dx::XMVECTOR aabbObjectPosition) const
	{
		// Reference: http://www.cse.chalmers.se/~uffe/vfc_bbox.pdf

		// Reference: https://www.gamedev.net/forums/topic/512123-fast--and-correct-frustum---aabb-intersection/
		const auto minCornerWS = dx::XMVectorAdd(aabb.GetMinimumCornerLS(), aabbObjectPosition);
		const auto maxCornerWS = dx::XMVectorAdd(aabb.GetMaximumCornerLS(), aabbObjectPosition);

		dx::XMVECTOR vmin = dx::XMVectorZero();
		for (size_t i = 0; i < 6; ++i) {
			// X axis 
			if (planes[i].x > 0) {
				vmin = dx::XMVectorSetX(vmin, dx::XMVectorGetX(minCornerWS));
			}
			else {
				vmin = dx::XMVectorSetX(vmin, dx::XMVectorGetX(maxCornerWS));
			}

			// Y axis 
			if (planes[i].y > 0) {
				vmin = dx::XMVectorSetY(vmin, dx::XMVectorGetY(minCornerWS));
			}
			else {
				vmin = dx::XMVectorSetY(vmin, dx::XMVectorGetY(maxCornerWS));
			}

			// Z axis 
			if (planes[i].z > 0) {
				vmin = dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(minCornerWS));
			}
			else {
				vmin = dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(maxCornerWS));
			}

			float dotMin = dx::XMVectorGetX(dx::XMVector3Dot(dx::XMLoadFloat4(&planes[i]), vmin));
			if (dotMin - planes[i].w > 0.f)
			{
				return false;
			}
		}
		return true;
	}

	// Slower version that also checks if AABB is entirely inside frustum
	const bool Frustum::DoesAABBIntersect(const AABB& aabb, const dx::XMVECTOR aabbObjectPosition, IntersectionType& intersectionType) const
	{
		// Reference: http://www.cse.chalmers.se/~uffe/vfc_bbox.pdf

		// Reference: https://www.gamedev.net/forums/topic/512123-fast--and-correct-frustum---aabb-intersection/
		intersectionType = IntersectionType::Inside;
		const auto minCornerWS = dx::XMVectorAdd(aabb.GetMinimumCornerLS(), aabbObjectPosition);
		const auto maxCornerWS = dx::XMVectorAdd(aabb.GetMaximumCornerLS(), aabbObjectPosition);

		dx::XMVECTOR vmin = dx::XMVectorZero();
		dx::XMVECTOR vmax = dx::XMVectorZero();
		for (size_t i = 0; i < 6; ++i) {
			// X axis 
			if (planes[i].x > 0) {
				vmin = dx::XMVectorSetX(vmin, dx::XMVectorGetX(minCornerWS));
				vmax = dx::XMVectorSetX(vmax, dx::XMVectorGetX(maxCornerWS));
			}
			else {
				vmin = dx::XMVectorSetX(vmin, dx::XMVectorGetX(maxCornerWS));
				vmax = dx::XMVectorSetX(vmax, dx::XMVectorGetX(minCornerWS));
			}

			// Y axis 
			if (planes[i].y > 0) {
				vmin = dx::XMVectorSetY(vmin, dx::XMVectorGetY(minCornerWS));
				vmax = dx::XMVectorSetY(vmax, dx::XMVectorGetY(maxCornerWS));
			}
			else {
				vmin = dx::XMVectorSetY(vmin, dx::XMVectorGetY(maxCornerWS));
				vmax = dx::XMVectorSetY(vmax, dx::XMVectorGetY(minCornerWS));
			}

			// Z axis 
			if (planes[i].z > 0) {
				vmin = dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(minCornerWS));
				vmax = dx::XMVectorSetZ(vmax, dx::XMVectorGetZ(maxCornerWS));
			}
			else {
				vmin = dx::XMVectorSetZ(vmin, dx::XMVectorGetZ(maxCornerWS));
				vmax = dx::XMVectorSetZ(vmax, dx::XMVectorGetZ(minCornerWS));
			}

			float dotMin = dx::XMVectorGetX(dx::XMVector3Dot(dx::XMLoadFloat4(&planes[i]), vmin));
			float dotMax = dx::XMVectorGetX(dx::XMVector3Dot(dx::XMLoadFloat4(&planes[i]), vmax));
			if (dotMin - planes[i].w > 0.f)
			{
				// Outside frustum
				intersectionType = IntersectionType::Outside;
				return false;
			}
			if (dotMax - planes[i].w >= 0.f)
			{
				// Intersection
				intersectionType = IntersectionType::Intersect;
			}
		}
		return true;
	}

	const bool Frustum::DoesSphereIntersect(const dx::XMVECTOR spherePosition, const float sphereRadius) const
	{
		for (size_t i = 0; i < PlaneCt; ++i)
		{
			// planes[i].w test is included in dot product
			float dot = dx::XMVectorGetX(dx::XMVector4Dot(dx::XMVectorSetW(spherePosition, 1.f), dx::XMLoadFloat4(&planes[i])));
			if (dot < -sphereRadius)
			{
				return false;
			}
		}
		return true;
	}

	const bool Frustum::DoesSphereIntersect(const dx::XMVECTOR spherePositionAndRadius) const
	{
		return DoesSphereIntersect(spherePositionAndRadius, dx::XMVectorGetW(spherePositionAndRadius));
	}

}