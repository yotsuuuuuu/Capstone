#include "CCapsuleCollider.h"
#include "CTransform.h"
#include "CActor.h"
#include <algorithm>
#include <limits>


bool CCapsuleCollider::OnCreate() {
	//UpdateCapsulePoints();
	//UpdateWorldAABB();
	auto actor = parent.lock();
	if (!actor)
		return;
	auto t = std::dynamic_pointer_cast<CActor>(actor)->GetComponent<CTransform>();
	if (!t)
		return;
	transform = t;

	return true;
}

bool CCapsuleCollider::IntersectingAABB(const AABB& aabb)
{
	Vec3 closestPointOnAABB = Vec3(
		std::max(aabb.min.x, std::min(transform.lock()->GetPosition().x, aabb.max.x)),
		std::max(aabb.min.y, std::min(transform.lock()->GetPosition().y, aabb.max.y)),
		std::max(aabb.min.z, std::min(transform.lock()->GetPosition().z, aabb.max.z))
	);

	float dist = GetDistanceToPoint(closestPointOnAABB);
	return dist <= 0.0f;
	//return dist <= radius;
}

bool CCapsuleCollider::IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	


}



Vec3 CCapsuleCollider::GetClosestPointOnCapsule(const Vec3& point) const
{

	return Vec3();
}

float CCapsuleCollider::GetDistanceToPoint(const Vec3& point) const
{
	return 0.0f;
}

Vec3 CCapsuleCollider::GetClosestPointOnTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c)
{


}

void CCapsuleCollider::UpdateCapsulePoints()
{
}

void CCapsuleCollider::UpdateWorldAABB()
{
}

CCapsuleCollider::~CCapsuleCollider()
{
}
