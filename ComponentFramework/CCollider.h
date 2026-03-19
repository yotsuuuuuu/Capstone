#pragma once
#include "Component.h"
#include <Vector.h>
#include "memory.h"

using namespace MATH;

//forward declaration
class CTransform;
class CCapsuleCollider; // just for now

enum class ColliderType {
	NONE = 0,
	CAPSULE,
	MESH,

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
	virtual void Update(const float dt) override;

	virtual ColliderType GetColliderType() const { return type; }

	virtual bool IntersectingWith(const CCollider& other) const = 0;
	virtual bool IntersectingCapsule(const CCapsuleCollider& capsule) const { return false; }
	//virtual bool IntersectingMesh(const CCollider& mesh) const { return false; }

	void SetDebugDraw(bool debug) { debugDraw = debug; }




};