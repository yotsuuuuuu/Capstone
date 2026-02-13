#include "CInput.h"
#include "CPhysics.h"
#include "CActor.h"
#include "CCamera.h"

bool CInput::OnCreate()
{
    if (isCreated) return true;

	auto a = parent.lock();

	if (!a) return false;

	auto T = std::dynamic_pointer_cast<CActor>(a)->GetComponent<CPhysics>();
	if (!T) return false;

	auto C = std::dynamic_pointer_cast<CActor>(a)->GetComponent<CCamera>();
	if (!C) return false;

	physics = T;
	camera = C;

	auto phys = physics.lock();
	if (!phys) return false;

	phys->SetHasGravity(false); // disable gravity for player controller for NOW until COLLISIONS
	phys->SetMass(5.0f);
	phys->SetDragCoefficient(0.3f);
	phys->SetVelocity(MATH::Vec3(0.0f, 0.0f, 0.0f)); // reset just in case

	isCreated = true;
	return true;
}

void CInput::OnDestroy()
{
	if (!isCreated) return;
}

void CInput::HandleKeyboardInput(const SDL_Event& event)
{
	//std::cout << event.key.key << std::endl;

	bool pressed = event.type == SDL_EVENT_KEY_DOWN; // true if key is pressed, false if released
	switch (event.key.key) {
	case SDLK_ESCAPE: // dont think this does anything yet
		if (pressed) { // only happens when its pressed, nothing when released
			mouseLookActive = !mouseLookActive;
		}
		break;
	case SDLK_W:
		moveForwardPressed = pressed;
		break;
	case SDLK_S:
		moveBackwardPressed = pressed;
		break;
	case SDLK_A:
		moveLeftPressed = pressed;
		break;
	case SDLK_D:
		moveRightPressed = pressed;
		break;
	case SDLK_LSHIFT:
		sprintPressed = pressed;
		break;
	case SDLK_SPACE:
		jumpPressed = pressed;
		break;
	default:
		break;
	}

}

void CInput::HandleMouseMotion(const SDL_Event& motion)
{
	if (!mouseLookActive) return; // do nothing when mouse look is not active

	float xOffset = static_cast<float>(-motion.motion.xrel); // negate x to invert mouse look (so moving mouse right looks right)
	float yOffset = static_cast<float>(-motion.motion.yrel); // negate y to invert mouse look (so moving mouse up looks up)

	xOffset *= mouseSensitivity;
	yOffset *= mouseSensitivity;

	yaw += xOffset;
	pitch += yOffset;

	// clamp
	if (pitch > 89.0f) { pitch = 89.0f; }
	if (pitch < -89.0f) { pitch = -89.0f; }

	UpdateCameraRotation();
}

void CInput::HandleMouseButton(const SDL_Event& button)
{
	// dunno if we need this for anything yet
	// TODO: (andres) figure out a use for mouse clicking and implement
}

void CInput::UpdateInput(const float dt)
{
	auto phys = physics.lock();

	auto cam = camera.lock();
	if (!phys || !cam) return;

	// would usually check if grounded but we dont care atm
	// CheckGrounded();

	MATH::Vec3 movementDirection = CalculateMovementDirection();
	float currentSpeed = moveSpeed * (sprintPressed ? sprintMultiplier : 1.0f);

	//std::cout << "Movement Direction: " << movementDirection.x << ", " << movementDirection.y << ", " << movementDirection.z << std::endl;

	//phys->SetVelocity(movementDirection * currentSpeed);
	MATH::Vec3 currentVelocity = phys->GetVelocity();

	if (MATH::VMath::mag(movementDirection) > VERY_SMALL) {

		currentVelocity = movementDirection * currentSpeed;
		//cam->SetNeedsUpdate(true); // flag camera to update its view matrix on next update
	}
	else 
	{
		currentVelocity = MATH::Vec3(0.0f, 0.0f, 0.0f);
	}

	phys->SetVelocity(currentVelocity);

	// update ubo here maybe
}

void CInput::UpdateCameraRotation()
{
	auto cam = camera.lock();
	auto phys = physics.lock();
	auto transform = std::dynamic_pointer_cast<CTransform>(phys);
	if (!cam || !phys) return;

	Quaternion yawRotation = QMath::angleAxisRotation(yaw, Vec3(0.0f, 1.0f, 0.0f));
	Quaternion pitchRotation = QMath::angleAxisRotation(pitch, Vec3(1.0f, 0.0f, 0.0f));

	transform->SetRotation(yawRotation * pitchRotation);
	//cam->SetNeedsUpdate(true); // flag camera to update its view matrix on next update

}

void CInput::CheckGrounded()
{
	// no floor so ignore this for now
	// TODO: (andres) implement raycast or collision check to set isGrounded properly. requires collisions to be implemented first
}

MATH::Vec3 CInput::CalculateMovementDirection() const
{
	auto cam = camera.lock();
	if (!cam) return MATH::Vec3(0.0f, 0.0f, 0.0f);

	MATH::Vec3 direction(0.0f, 0.0f, 0.0f);

	MATH::Vec3 front = cam->GetFrontVector();
	MATH::Vec3 right = cam->GetRightVector();

	// commented out for now since we flying
	//front.y = 0.0f; // ignore vertical component for movement (so we dont fly when looking up )

	if (MATH::VMath::mag(front) > 0.0f) {
		front = MATH::VMath::normalize(front);
	}

	if (moveForwardPressed) { direction += front; }
	if (moveBackwardPressed) { direction -= front; }
	if (moveLeftPressed) { direction -= right; }
	if (moveRightPressed) { direction += right; }

	// normalize if moving diagonally to prevent faster movement
	if (MATH::VMath::mag(direction) > 0.0f) {
		direction = MATH::VMath::normalize(direction);
	}

	return direction;

}
