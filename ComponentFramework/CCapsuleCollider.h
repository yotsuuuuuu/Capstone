#pragma once

#pragma once
#include "CCollider.h"
#include "CoreStructs.h"

class CTransform;

class CCapsuleCollider : public CCollider
{
	float radius = 0.5f;
	float height = 2.0f;

	WeakRef<CTransform> transform;
	
	Vec3 capA; // tip
	Vec3 capB; // base
	Vec3 axis; // normalized axis from bottom to top
	// center is from transform (+ offset)

	//float penetrationDepth = 0.0f; // for collision response

public:
	CCapsuleCollider(Ref<Component> parent_, Vec3 offset_ = Vec3(), float radius_ = 0.5f, float height_ = 2.0f) :
		CCollider(parent_, ColliderType::CAPSULE, Vec3()),
		radius(radius_), height(height_)
	{
	}

	~CCapsuleCollider();

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float dt) override;


	//bool IntersectingWith(const CCollider& other) const override;
	//bool IntersectingCapsule(const CCapsuleCollider& capsule);

	virtual CollisionInfo IntersectingWith(const CCollider& other) const override;
	virtual CollisionInfo IntersectingCapsule(const CCapsuleCollider& capsule) const override;

	virtual CollisionInfo IntersectingAABB(const AABB& aabb) override;

	virtual bool SimpleIntersectingAABB(const AABB& aabb) override; // just check if the capsule's AABB is intersecting with the given AABB, used for broad phase
	virtual MeshCollisionInfo IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) override;

	float GetRadius() { return radius; }
	float GetHeight() { return height; }
	const Vec3& GetCapA() { return capA; }
	const Vec3& GetCapB() { return capB; }
	const Vec3& GetAxis() { return axis; }

	
	CollisionInfo IntersectingSphereTriangle(const Vec3& sphereCenter, float sphereRadius, const Vec3& v0, const Vec3& v1, const Vec3& v2) const;
	CollisionInfo IntersectingCapsuleTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) const;
	AABB GetAABB() const; 
};