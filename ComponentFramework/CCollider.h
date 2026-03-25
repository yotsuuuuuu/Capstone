#pragma once
#include "Component.h"
#include <Vector.h>
#include "memory.h"
#include "CoreStructs.h"

using namespace MATH;

//forward declaration
class CTransform;
class CCapsuleCollider; // just for now

enum class ColliderType {
	NONE = 0,
	CAPSULE,
	MESH,
	AABB
};

struct CollisionInfo {
	Vec3 point; // point of collision
	Vec3 normal; // normal at the point of collision
	float penetrationDepth = 0.0f; // how deep the collision is, used for collision response
	bool isColliding = false; // whether a collision is happening or not
};

struct MeshCollisionInfo : public CollisionInfo {
	uint32_t triangleIndex; // index of the triangle that was hit
};

class CCollider : public Component
{
protected:
	ColliderType type = ColliderType::NONE;

	// use parents transform;
	WeakRef<CTransform> transform;

	Vec3 offset; // so we can offset (move up/down)

	bool debugDraw = false; 

	
public:
	CCollider(Ref<Component> parent_, ColliderType type_, Vec3 offset_ = Vec3()) : Component(parent_), type(type_), offset(offset_) 
	{
		
	}

	virtual ~CCollider() = default;
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) = 0;

	virtual ColliderType GetColliderType() const { return type; }

	virtual bool SimpleIntersectingAABB(const AABB& aabb) const = 0; // just check if the collider's AABB is intersecting with the given AABB, used for broad phase
	virtual CollisionInfo IntersectingAABB(const AABB& aabb) const = 0;

	virtual CollisionInfo IntersectingWith(const CCollider& other) const = 0;
	virtual CollisionInfo IntersectingCapsule(const CCapsuleCollider& capsule) const = 0;
	virtual MeshCollisionInfo IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) const = 0;

	//virtual bool IntersectingMesh(const CCollider& mesh) const { return false; }

	void SetDebugDraw(bool debug) { debugDraw = debug; }

	// helper
	Vec3 ClosestPointOnSegment(const Vec3& a, const Vec3& b, const Vec3& point) const; // segment / axis
	Vec3 ClosestPointOnTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& point) const;
	Vec3 ClosestPointOnAABB(const AABB& aabb, const Vec3& point) const;
	float SquaredDistanceToSegment(const Vec3& a, const Vec3& b, const Vec3& point) const;




};