#pragma once
#include "Component.h"
#include <Vector.h>
#include <Quaternion.h>
class CTransform : public Component
{
protected:
	MATH::Vec3 position;
	MATH::Quaternion rotation;
	MATH::Vec3 scale;
public:
	CTransform(Ref<Component> parent_ = nullptr) : Component(parent_),
		position(0.0f, 0.0f, 0.0f), rotation(), scale(1.0f, 1.0f, 1.0f) {}
	CTransform(Ref<Component> parent_, const MATH::Vec3& pos, const MATH::Quaternion& rot, const MATH::Vec3& scale_) :
		Component(parent_), position(pos), rotation(rot), scale(scale_) {}
	virtual ~CTransform() {}

	virtual bool OnCreate() override { return true; }
	virtual void OnDestroy() override {}
	virtual void Update(const float dt) override {}

	inline MATH::Vec3 GetPosition() const { return position; }
	inline void SetPosition(const MATH::Vec3& pos) { position = pos; }	
	inline MATH::Quaternion GetRotation() const { return rotation; }
	inline void SetRotation(const MATH::Quaternion& rot) { rotation = rot; }
	inline MATH::Vec3 GetScale() const { return scale; }
	inline void SetScale(const MATH::Vec3& scl) { scale = scl; }

	MATH::Matrix4 GetTransformMatrix() const;
};

