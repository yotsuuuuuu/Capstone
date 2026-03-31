#include "CCollider.h"
#include "CPhysics.h"
#include "CActor.h"



//bool CCollider::OnCreate()
//{
//    if (isCreated) return true;
//
//    auto actor = parent.lock();
//    if (!actor) return false;
//
//    // Ensure we have a transform component
//    auto t = std::dynamic_pointer_cast<CActor>(actor)->GetComponent<CTransform>();
//    if (!t) {
//        // Can't have collider without transform
//        return false;
//    }
//
//	transform = t;
//
//    isCreated = true;
//    return true;
//}

void CCollider::OnDestroy()
{
    isCreated = false;
}

void CCollider::Update(const float dt)
{
    // For now, collider doesn't need to do anything on update
    // In the future, we might want to update the collider's position based on the transform
}

Vec3 CCollider::ClosestPointOnSegment(const Vec3& a, const Vec3& b, const Vec3& point) const
{
    Vec3 ab = b - a; // vector from a to b (the segment)

    // project point onto the line defined by a and b, then normalize by the length of ab to get t
    float t = VMath::dot(point - a, ab) / VMath::dot(ab, ab);

    t = std::clamp(t, 0.0f, 1.0f); // clamp t to the range [0, 1] to ensure the closest point is on the segment

    return a + (ab * t); // original point + the vectros * distance = point
}

Vec3 CCollider::ClosestPointOnTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& point) const
{
	// 3 points of the triangle and the point we want to find the closest point to
	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 ap = point - a;
    
	float d1 = VMath::dot(ab, ap);
	float d2 = VMath::dot(ac, ap);

	if (d1 <= 0.0f && d2 <= 0.0f) return a; // point is in the region of vertex a

	Vec3 bp = point - b;
	float d3 = VMath::dot(ab, bp);
	float d4 = VMath::dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3) return b; // point is in the region of vertex b

	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) { // point is in the region of ab
		float v = d1 / (d1 - d3); // distance from a to the closest point on the edge ab, normalized by the length of ab
        return a + (ab * v);
	}
    
	Vec3 cp = point - c;
	float d5 = VMath::dot(ab, cp);
	float d6 = VMath::dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6) return c; // point is in the region of vertex c

	float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) { // point is in the region of ac
		float w = d2 / (d2 - d6); // distance from a to the closest point on the edge ac, normalized by the length of ac
		return a + (ac * w);
    }

	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) { // point is in the region of bc
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6)); // distance from b to the closest point on the edge bc, normalized by the length of bc
		return b + ((c - b) * w);
    }

	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return a + (ab * v) + (ac * w); // point is inside the face region of the triangle

}

Vec3 CCollider::ClosestPointOnAABB(const AABB& aabb, const Vec3& point) const
{
    Vec3 closestPoint;
    closestPoint.x = std::clamp(point.x, aabb.min.x, aabb.max.x);
    closestPoint.y = std::clamp(point.y, aabb.min.y, aabb.max.y);
    closestPoint.z = std::clamp(point.z, aabb.min.z, aabb.max.z);

    return closestPoint;
}

float CCollider::SquaredDistanceToSegment(const Vec3& a, const Vec3& b, const Vec3& point) const
{
    Vec3 ab = b - a;
    Vec3 ap = point - a;

    float t = VMath::dot(ap, ab) / VMath::dot(ab, ab);
    
    if (t < 0.0f) {
		return VMath::dot(ap, ap); // distance from point to a
    }
    if (t > 1.0f) {
		return VMath::dot(point - b, point - b); // distance from point to b
    }
    //t = std::clamp(t, 0.0f, 1.0f);

    Vec3 closestPoint = a + ab * t;
    Vec3 diff = point - closestPoint;

    return VMath::dot(diff, diff);
}

void CCollider::ApplyCollisionResponse(const MeshCollisionInfo& info)
{
    if (auto phys = physics.lock()) {

        phys->SetPosition(phys->GetPosition() + info.normal * info.penetrationDepth);
        //std::cout << info.penetrationDepth << std::endl;
        bool isGround = VMath::dot(info.normal, Vec3(0, 1, 0)) > 0.3f;

        if (isGround) 
        {
            phys->SetIsGrounded(true);
            Vec3 vel = phys->GetVelocity();
            vel.y = 0.0f;
            phys->SetVelocity(vel);
            std::cout << "vel resest" << std::endl;
        }
        else {
            phys->SetIsGrounded(false);
        }

    }
}




