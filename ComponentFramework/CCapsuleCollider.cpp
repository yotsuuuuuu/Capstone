#include "CCapsuleCollider.h"
#include "CPhysics.h"
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
	auto p = std::dynamic_pointer_cast<CActor>(actor)->GetComponent<CPhysics>();
	if (!p)	return false;
	physics = p;

	cylinderHeight = height - radius * 2.0f;  // The straight cylinder part

	UpdateCapsulePoints();

	return true;
}

void CCapsuleCollider::OnDestroy()
{
	// nothing to clean up for now
}

void CCapsuleCollider::Update(const float dt)
{
	UpdateCapsulePoints();
}


CollisionInfo CCapsuleCollider::IntersectingWith(const CCollider& other) const
{
	return CollisionInfo();
}

CollisionInfo CCapsuleCollider::IntersectingCapsule(const CCapsuleCollider& capsule) const
{
	return CollisionInfo();
}



CollisionInfo CCapsuleCollider::IntersectingAABB(const AABB& aabb) const// capsul aabb
{
	CollisionInfo info;

	Vec3 axis = capA - capB;
	float axislength = VMath::mag(axis);
	float radius = this->radius;

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

MeshCollisionInfo CCapsuleCollider::IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Vec2& chunkPosition) const
{
	MeshCollisionInfo info;

	if (vertices.empty() || indices.empty()) {
		return info; // no collision if mesh is empty
	}

	float maxPenetration = 0.0f;

	for (size_t i = 0; i < indices.size(); i += 3) {

		// local vertices of the triangle
		const Vertex& v0 = vertices[indices[i]];
		const Vertex& v1 = vertices[indices[i + 1]];
		const Vertex& v2 = vertices[indices[i + 2]];

		Vec3 worldV0 = v0.pos + Vec3(chunkPosition.x, 0.0f, chunkPosition.y);
		Vec3 worldV1 = v1.pos + Vec3(chunkPosition.x, 0.0f, chunkPosition.y);
		Vec3 worldV2 = v2.pos + Vec3(chunkPosition.x, 0.0f, chunkPosition.y);



		CollisionInfo triInfo = IntersectingCapsuleTriangle(worldV0, worldV1, worldV2);

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

bool CCapsuleCollider::SimpleIntersectingAABB(const AABB& aabb) const
{
	AABB capsuleAABB = GetAABB();

	if (capsuleAABB.max.x < aabb.min.x || capsuleAABB.min.x > aabb.max.x) return false;
	if (capsuleAABB.max.y < aabb.min.y || capsuleAABB.min.y > aabb.max.y) return false;
	if (capsuleAABB.max.z < aabb.min.z || capsuleAABB.min.z > aabb.max.z) return false;

	return true;
}



void CCapsuleCollider::UpdateCapsulePoints()
{
	auto phys = physics.lock();
	if (!phys) return;

	Vec3 center = phys->GetPosition() + offset;

	cylinderHeight = height - radius * 2.0f;
	if (cylinderHeight < 0.0f) cylinderHeight = 0.0f;

	halfCylinder = cylinderHeight * 0.5f;

	// bottom of capsule (feet)
	capB = center - Vec3(0.0f, halfCylinder, 0.0f);

	// top of capsule (head)
	capA = center + Vec3(0.0f, halfCylinder, 0.0f);

	axis = VMath::normalize(capA - capB);
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
	//std::cout << "Closest Point: " << closestPoint.x << " " << closestPoint.y << " " << closestPoint.z << std::endl;
	Vec3 delta = sphereCenter - closestPoint;

	float distance = VMath::mag(delta);

	if (distance < sphereRadius) {
		info.isColliding = true;
		info.point = closestPoint;

		info.penetrationDepth = distance < VERY_SMALL ? sphereRadius + 1.0f : sphereRadius - distance;
		info.normal = distance < VERY_SMALL ? delta : VMath::normalize(delta);
	}
	return info;
}

CollisionInfo CCapsuleCollider::IntersectingCapsuleTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) const
{
	CollisionInfo info;

	float maxPenetration = 0.0f;

	float axisLength = VMath::mag(axis);
	if (axisLength < VERY_SMALL) {
		// degenerate capsule (sphere)
		return IntersectingSphereTriangle(capB, radius, v0, v1, v2);
	}

	Vec3 axisDir = axis / axisLength;

	int numSamples = std::max(3, static_cast<int>(axisLength / radius) + 2); // more samples for longer capsules

	for (int i = 0; i <= numSamples; ++i) {
		float t = static_cast<float>(i) / numSamples;
		Vec3 sphereCenter = capB + axisDir * (t * axisLength); // point on the capsule axis
		CollisionInfo sphereInfo = IntersectingSphereTriangle(sphereCenter, radius, v0, v1, v2);
		if (sphereInfo.isColliding && sphereInfo.penetrationDepth > maxPenetration) {
			maxPenetration = sphereInfo.penetrationDepth;
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




