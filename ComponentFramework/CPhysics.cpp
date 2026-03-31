#include "CPhysics.h"

void CPhysics::ApplyForce(MATH::Vec3 force)
{
	// F = m * a
	// a = F / m
	acceleration += force / mass;
	//acceleration += acceleration;
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
	//std::cout << "VELOCITY: " << velocity.x << ", " << velocity.y << ", " << velocity.z << std::endl;

	if (velocity.y <= VERY_SMALL) { velocity.y = 0.0f; }

	if (hasGravity && !isGrounded){ // only apply gravity when not on the ground
		const MATH::Vec3 gravity(0.0f, -9.81f, 0.0f);
		acceleration += gravity;
	}

	ApplyDragForce();
	velocity += acceleration * deltaTime;

	if (MATH::VMath::mag(velocity) >= VERY_SMALL) { needsUBOupdate = true; } // if moving then update UBO
	position += velocity * deltaTime;

	acceleration = MATH::Vec3(0.0f, 0.0f, 0.0f); // Reset acceleration after each update

}
