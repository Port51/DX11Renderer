#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class AABB;

	// Frustum that can be in any coordinate space - usually world-space or view-space
	struct Frustum
	{
	public:
		enum IntersectionType { Intersect, Inside, Outside };
	public:
		Frustum();
		Frustum(const std::vector<dx::XMFLOAT4>& planes);

	public:
		// Typically, the matrix will be either the projection matrix or view-projection matrix
		void UpdatePlanesFromMatrix(const dx::XMMATRIX& matrix);
		void UpdatePlanesFromViewSpaceCorners(const dx::XMVECTOR& frustumCorners, const float nearPlane, const float farPlane);
		const dx::XMVECTOR GetGPUFrustumPlaneDir() const;

	public:
		const bool DoesAABBIntersect(const AABB& aabb, const dx::XMVECTOR& aabbObjectPosition) const;
		const bool DoesAABBIntersect(const AABB& aabb, const dx::XMVECTOR& aabbObjectPosition, IntersectionType& intersectionType) const;
		const bool DoesSphereIntersect(const dx::XMVECTOR& spherePosition, const float sphereRadius) const;
		// spherePosition.w must be equal to radius
		const bool DoesSphereIntersect(const dx::XMVECTOR& spherePositionAndRadius) const;

	public:
		// XYZ = normal, W = distance
		std::vector<dx::XMFLOAT4> planes;

	private:
		const int PlaneCt = 6;
	};
}