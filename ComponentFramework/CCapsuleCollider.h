#pragma once

#pragma once
#include "CCollider.h"
#include "CoreStructs.h"

class CTransform;

class CCapsuleCollider : public CCollider
{
	float radius = 0.5f;
	float height = 2.0f;
	float cylinderHeight = height - radius * 2.0f;  // The straight cylinder part
	//float cameraHeight = cylinderHeight * 0.7f; // for camera collision

	//WeakRef<CTransform> transform;
	
	Vec3 capA; // tip
	Vec3 capB; // base
	Vec3 axis; // normalized axis from bottom to top
	// center is from transform (+ offset)

	//float penetrationDepth = 0.0f; // for collision response

public:
	CCapsuleCollider(Ref<Component> parent_, float radius_ = 0.5f, float height_ = 5.0f, Vec3 offset_ = Vec3()) :
		CCollider(parent_, ColliderType::CAPSULE, offset_),
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

	virtual CollisionInfo IntersectingAABB(const AABB& aabb) const override;

	virtual bool SimpleIntersectingAABB(const AABB& aabb) const override; // just check if the capsule's AABB is intersecting with the given AABB, used for broad phase
	virtual MeshCollisionInfo IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Vec2& chunkPosition) const override;

	float GetRadius() { return radius; }
	float GetHeight() { return height; }
	const Vec3& GetCapA() { return capA; }
	const Vec3& GetCapB() { return capB; }
	const Vec3& GetAxis() { return axis; }

	const void SetCapA(const Vec3& newCapA) { capA = newCapA; }
	const void SetCapB(const Vec3& newCapB) { capB = newCapB; }

	void UpdateCapsulePoints();
	CollisionInfo IntersectingSphereTriangle(const Vec3& sphereCenter, float sphereRadius, const Vec3& v0, const Vec3& v1, const Vec3& v2) const;
	CollisionInfo IntersectingCapsuleTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) const;
	AABB GetAABB() const; 
};