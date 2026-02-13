#include "CPhysics.h"

void CPhysics::ApplyForce(MATH::Vec3 force)
{
	// F = m * a
	// a = F / m
	MATH::Vec3 acceleration = force / mass;
	acceleration += acceleration;
	velocity += acceleration;
}

void CPhysics::ApplyImpulse(MATH::Vec3 impulse)
{
	// Impulse = m * deltaV
	// deltaV = Impulse / m
	velocity += impulse / mass;
}

void CPhysics::ApplyDragForce()
{
	// F = -v * dragCoefficient
	MATH::Vec3 dragForce = -velocity * dragCoefficient;
	ApplyForce(dragForce);
}

void CPhysics::Update(const float deltaTime)
{
	if (hasGravity) {
		const MATH::Vec3 gravity(0.0f, -9.81f, 0.0f);
		acceleration += gravity;
	}

	ApplyDragForce();
	velocity += acceleration * deltaTime;

	if (MATH::VMath::mag(velocity) >= VERY_SMALL) { needsUBOupdate = true; } // if moving then update UBO
	position += velocity * deltaTime;

	//acceleration = MATH::Vec3(0.0f, 0.0f, 0.0f); // Reset acceleration after each update

}
