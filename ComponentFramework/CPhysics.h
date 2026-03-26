#pragma once
#include "CTransform.h"
class CPhysics : public CTransform {
	
private:
	float mass;
	float dragCoefficient;
	MATH::Vec3 velocity;
	MATH::Vec3 acceleration;

	bool hasGravity = true;
	bool isGrounded = false;

public:
	CPhysics(Ref<Component> parent_) :
		CTransform(parent_),
		mass(1.0f),
		dragCoefficient(0.01f),
		velocity(MATH::Vec3()),
		acceleration(MATH::Vec3()),
		hasGravity(true) {}

	CPhysics(Ref<Component> parent_, float mass_, MATH::Vec3 velocity_, MATH::Vec3 acceleration_, bool hasGravity_) :
		CTransform(parent_),
		mass(mass_),
		dragCoefficient(0.01f),
		velocity(velocity_),
		acceleration(acceleration_),
		hasGravity(hasGravity_){}

	~CPhysics() {}
	bool OnCreate() { return true; }
	void OnDestroy() {}

	void SetMass(float mass_) { mass = mass_; }
	void SetPosition(MATH::Vec3 pos_) { position = pos_; }
	void SetVelocity(MATH::Vec3 velocity_) { velocity = velocity_; }
	void SetAcceleration(MATH::Vec3 acceleration_) { acceleration = acceleration_; }
	void SetHasGravity(bool hasGravity_) { hasGravity = hasGravity_; }
	void SetDragCoefficient(float dragCoefficient_) { dragCoefficient = dragCoefficient_; }
	//void SetRotation(MATH::Quaternion rot_) { rotation = rot_; }
	void SetIsGrounded(bool grounded_) { isGrounded = grounded_; }
	bool IsGrounded() const { return isGrounded; }
	float GetMass() { return mass; }
	MATH::Vec3 GetVelocity() { return velocity; }
	MATH::Vec3 GetAcceleration() { return acceleration; }

	void ApplyForce(MATH::Vec3 force);
	void ApplyImpulse(MATH::Vec3 impulse);
	void ApplyDragForce();

	virtual void Update(const float deltaTime);

};

