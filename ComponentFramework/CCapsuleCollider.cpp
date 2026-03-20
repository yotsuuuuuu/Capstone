#include "CCapsuleCollider.h"
#include "CTransform.h"
#include "CActor.h"
#include <algorithm>
#include <limits>


CCapsuleCollider::~CCapsuleCollider()
{
}

bool CCapsuleCollider::OnCreate() {
	//UpdateCapsulePoints();
	//UpdateWorldAABB();
	auto actor = parent.lock();
	if (!actor)	return false;
	auto t = std::dynamic_pointer_cast<CActor>(actor)->GetComponent<CTransform>();
	if (!t)	return false;
	transform = t;

	return true;
}

void CCapsuleCollider::OnDestroy()
{
	// nothing to clean up for now
}

void CCapsuleCollider::Update(const float dt)
{
}


CollisionInfo CCapsuleCollider::IntersectingWith(const CCollider& other) const
{
	return CollisionInfo();
}

CollisionInfo CCapsuleCollider::IntersectingCapsule(const CCapsuleCollider& capsule) const
{
	return CollisionInfo();
}

CollisionInfo CCapsuleCollider::IntersectingAABB(const AABB& aabb) // capsul aabb
{
	CollisionInfo info;

	Vec3 axis = capA - capB;
	float axislength = VMath::mag(axis);

	if (axislength < VERY_SMALL) {
		// degenerate capsule (sphere)
		Vec3 closestPoint = ClosestPointOnAABB(aabb, capA);
		float distanceSq = VMath::dot(closestPoint - capA, closestPoint - capA);
		info.isColliding = distanceSq <= radius * radius;
		info.penetrationDepth = radius - sqrt(distanceSq);
		info.point = closestPoint;
		info.normal = VMath::normalize(closestPoint - capA); // normal from capsule center to closest point (sphere)
		return info;
	}

	Vec3 axisDir = axis / axislength; // normalize the axis


	const int numSamples = 10; // number of samples along the capsule axis

	float minDistanceSq = std::numeric_limits<float>::max();
	Vec3 intersectionPoint;
	Vec3 capsulePoint;

	for (int i = 0; i <= numSamples; ++i) {

		float t = axislength * (i / static_cast<float>(numSamples));

		Vec3 pointOnCapsule = capB + axisDir * t; // point on the capsule axis

		Vec3 closestPoint = ClosestPointOnAABB(aabb, pointOnCapsule); // closest point on AABB to this point

		float distanceSq = VMath::dot(closestPoint - pointOnCapsule, closestPoint - pointOnCapsule); // squared distance

		if (distanceSq < minDistanceSq) { // keep track of the closest point and distance
			minDistanceSq = distanceSq;
			intersectionPoint = closestPoint;
			capsulePoint = pointOnCapsule;
		}
	}

	info.isColliding = minDistanceSq <= radius * radius;
	info.penetrationDepth = radius - sqrt(minDistanceSq);
	info.point = intersectionPoint;
	// collision normal is from the closest point on the capsule axis to the intersection point, 
	// projected onto the plane perpendicular to the capsule axis
	info.normal = VMath::normalize(intersectionPoint - capsulePoint - VMath::dot(intersectionPoint - capsulePoint, axisDir) * axisDir);
	return info;
}

MeshCollisionInfo CCapsuleCollider::IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	MeshCollisionInfo info;

	if (vertices.empty() || indices.empty()) {
		return info; // no collision if mesh is empty
	}

	float maxPenetration = 0.0f;

	for (size_t i = 0; i < indices.size(); i += 3) {

		const Vec3& v0 = vertices[indices[i]].pos;
		const Vec3& v1 = vertices[indices[i + 1]].pos;
		const Vec3& v2 = vertices[indices[i + 2]].pos;

		CollisionInfo triInfo = IntersectingCapsuleTriangle(v0, v1, v2);

		if (triInfo.isColliding && triInfo.penetrationDepth > maxPenetration) {
			maxPenetration = triInfo.penetrationDepth;
			info.isColliding = true;
			info.penetrationDepth = triInfo.penetrationDepth;
			info.point = triInfo.point;
			info.normal = triInfo.normal;
			info.triangleIndex = static_cast<uint32_t>(i / 3);
		}
	}

	return info;
}

bool CCapsuleCollider::SimpleIntersectingAABB(const AABB& aabb)
{
	AABB capsuleAABB = GetAABB();

	if (capsuleAABB.max.x < aabb.min.x || capsuleAABB.min.x > aabb.max.x) return false;
	if (capsuleAABB.max.y < aabb.min.y || capsuleAABB.min.y > aabb.max.y) return false;
	if (capsuleAABB.max.z < aabb.min.z || capsuleAABB.min.z > aabb.max.z) return false;

	return true;
}



CollisionInfo CCapsuleCollider::IntersectingSphereTriangle(const Vec3& sphereCenter, float sphereRadius, const Vec3& v0, const Vec3& v1, const Vec3& v2) const
{

	CollisionInfo info;

	Vec3 edge1 = v1 - v0;
	Vec3 edge2 = v2 - v0;
	Vec3 normal = VMath::normalize(VMath::cross(edge1, edge2));

	float distToPlane = VMath::dot(sphereCenter - v0, normal);	
	if (std::fabs(distToPlane) > sphereRadius) {
		return info; // no collision
	}

	Vec3 projectedCenter = sphereCenter - distToPlane * normal;

	Vec3 closestPoint = ClosestPointOnTriangle(v0, v1, v2, projectedCenter);

	Vec3 delta = projectedCenter - closestPoint;

	float distance = VMath::mag(delta);

	if (distance < sphereRadius) {
		info.isColliding = true;
		info.penetrationDepth = sphereRadius - distance;
		info.point = closestPoint;
		info.normal = VMath::normalize(delta);
	}
	return info;
}

CollisionInfo CCapsuleCollider::IntersectingCapsuleTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) const
{
	CollisionInfo info;

	float minPenetration = std::numeric_limits<float>::max(); // or 0

	Vec3 axis = capA - capB;

	float axisLength = VMath::mag(axis);
	if (axisLength < VERY_SMALL) {
		// degenerate capsule (sphere)
		return IntersectingSphereTriangle(capB, radius, v0, v1, v2);
	}

	Vec3 axisDir = axis / axisLength;

	int numSamples = std::max(3, static_cast<int>(axisLength / radius) + 2); // more samples for longer capsules

	for (int i = 0; i <= numSamples; ++i) {
		float t = axisLength * (i / static_cast<float>(numSamples));
		Vec3 sphereCenter = capB + axisDir * t * axisLength; // point on the capsule axis
		CollisionInfo sphereInfo = IntersectingSphereTriangle(sphereCenter, radius, v0, v1, v2);
		if (sphereInfo.isColliding && sphereInfo.penetrationDepth < minPenetration) {
			minPenetration = sphereInfo.penetrationDepth;
			info = sphereInfo;
		}
	}

	return info;
}

AABB CCapsuleCollider::GetAABB() const
{
	AABB capsuleAABB;

	capsuleAABB.min = capB;
	capsuleAABB.max = capB;

	capsuleAABB.min.x = std::min(capsuleAABB.min.x, capA.x);
	capsuleAABB.min.y = std::min(capsuleAABB.min.y, capA.y);
	capsuleAABB.min.z = std::min(capsuleAABB.min.z, capA.z);

	capsuleAABB.max.x = std::max(capsuleAABB.max.x, capA.x);
	capsuleAABB.max.y = std::max(capsuleAABB.max.y, capA.y);
	capsuleAABB.max.z = std::max(capsuleAABB.max.z, capA.z);

	capsuleAABB.min.x -= radius;
	capsuleAABB.min.y -= radius;
	capsuleAABB.min.z -= radius;

	capsuleAABB.max.x += radius;
	capsuleAABB.max.y += radius;
	capsuleAABB.max.z += radius;

	return capsuleAABB;
}




