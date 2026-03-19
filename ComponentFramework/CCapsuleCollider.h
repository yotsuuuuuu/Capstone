#pragma once

#pragma once
#include "Component.h"
#include "CoreStructs.h"

class CTransform;

class CCapsuleCollider : public Component
{
	float radius = 0.5f;
	float height = 2.0f;

	WeakRef<CTransform> transform;
	
	Vec3 capA;
	Vec3 capB;
	Vec3 axis; // normalized axis from bottom to top
	// center is from transform (+ offset)
	AABB worldAABB;

public:
	CCapsuleCollider(Ref<Component> parent_, Vec3 offset_ = Vec3(), float radius_ = 0.5f, float height_ = 2.0f) :
		Component(parent_), radius(radius_), height(height_)
	{
	}

	~CCapsuleCollider();

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float dt) override;



	//bool IntersectingWith(const CCollider& other) const override;
	bool IntersectingCapsule(const CCapsuleCollider& capsule);

	bool IntersectingChunk(const TerrainChunkData& chunk) ;
	bool IntersectingAABB(const AABB& aabb) ;
	bool IntersectingMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	float GetRadius() { return radius; }
	float GetHeight() { return height; }
	const Vec3& GetCapA() { return capA; }
	const Vec3& GetCapB() { return capB; }
	const Vec3& GetAxis() { return axis; }
	const AABB& GetWorldAABB() { return worldAABB; }

	Vec3 GetClosestPointOnCapsule(const Vec3& point) const;
	float GetDistanceToPoint(const Vec3& point) const;
	Vec3 GetClosestPointOnTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c);

private:
	// can either update the capsule or when checking tranform it along the players transform,
	void UpdateCapsulePoints(); 
	void UpdateWorldAABB();

};