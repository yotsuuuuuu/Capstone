#include "CInput.h"
#include "CPhysics.h"
#include "CActor.h"
#include "CCamera.h"
#include "CustomSDLEvents.h"

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

	phys->SetHasGravity(true); // disable gravity for player controller for NOW until COLLISIONS
	phys->SetMass(1.0f);
	phys->SetDragCoefficient(0.3f);
	phys->SetVelocity(MATH::Vec3(0.0f, 0.0f, 0.0f)); // reset just in case

	isCreated = true;
	return true;
}

void CInput::OnDestroy()
{
	if (!isCreated) return;
}

// TODO: (andres) refactor handle input stuff to be one call on scene,
// TODO: (andres) maybe make playerComponent that can update all required components in one call.
// TODO: (andres) click to enable mouse lock (?)

void CInput::HandleKeyboardInput(const SDL_Event& event)
{
	//std::cout << event.key.key << std::endl;
	if (event.type == CustomEvent::AUDIO_MENU_EVENT) {
		if (event.user.code == 1)       // menu opened
			mouseLookActive = false;
		else                            // menu closed
			mouseLookActive = true;
	}
	bool pressed = event.type == SDL_EVENT_KEY_DOWN; // true if key is pressed, false if released
	switch (event.key.key) {
	case SDLK_ESCAPE: // dont think this does anything yet
		if (pressed) { // only happens when its pressed, nothing when released
			//mouseLookActive = !mouseLookActive;
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
	case SDLK_V:
		noclip = pressed;
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
	if (motion.type != SDL_EVENT_MOUSE_MOTION) return;
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

void CInput::Update(const float dt)
{
	auto phys = physics.lock();

	if (!phys) return;

	// would usually check if grounded but we dont care atm
	// CheckGrounded();

	MATH::Vec3 movementDirection = CalculateMovementDirection();
	float currentSpeed = moveSpeed * (sprintPressed && phys->IsGrounded() ? sprintMultiplier : 1.0f);

	//std::cout << "Movement Direction: " << movementDirection.x << ", " << movementDirection.y << ", " << movementDirection.z << std::endl;

	//phys->SetVelocity(movementDirection * currentSpeed);
	MATH::Vec3 currentVelocity = phys->GetVelocity();

	if (MATH::VMath::mag(movementDirection) > VERY_SMALL) {
		currentVelocity.x = movementDirection.x * currentSpeed;
		currentVelocity.z = movementDirection.z * currentSpeed;
	}
	else 
	{
		// player decceleration
		if (phys->IsGrounded()) {
			currentVelocity = currentVelocity / 1.3f;
		}
	}

	phys->SetVelocity(currentVelocity);

	// apply jump AFTER movement
 	if (jumpPressed && phys->IsGrounded()) {
		phys->ApplyImpulse(Vec3(0.0f, jumpStrength, 0.0f));
		phys->SetIsGrounded(false);
	} 
}

void CInput::UpdateCameraRotation()
{
	auto phys = physics.lock();
	//auto transform = std::dynamic_pointer_cast<CTransform>(phys);
	if (!phys) return;

	Quaternion yawRotation = QMath::angleAxisRotation(yaw, Vec3(0.0f, 1.0f, 0.0f));
	Quaternion pitchRotation = QMath::angleAxisRotation(pitch, Vec3(1.0f, 0.0f, 0.0f));

	phys->SetRotation(yawRotation * pitchRotation);

}


MATH::Vec3 CInput::CalculateMovementDirection() const
{
	auto cam = camera.lock();
	if (!cam) return MATH::Vec3(0.0f, 0.0f, 0.0f);

	MATH::Vec3 direction(0.0f, 0.0f, 0.0f);

	MATH::Vec3 front = cam->GetFrontVector();
	MATH::Vec3 right = cam->GetRightVector();

	MATH::Vec3 up = cam->GetUpVector();
	// commented out for now since we flying

	if (MATH::VMath::mag(front) > 0.0f) {
		front = MATH::VMath::normalize(front);
	}

	if (moveForwardPressed) { direction += front; }
	if (moveBackwardPressed) { direction -= front; }
	if (moveLeftPressed) { direction -= right; }
	if (moveRightPressed) { direction += right; }

	if (!moveForwardPressed && !moveBackwardPressed && !moveLeftPressed && !moveRightPressed) {

	}


	// normalize if moving diagonally to prevent faster movement
	if (MATH::VMath::mag(direction) > 0.0f) {
		direction = MATH::VMath::normalize(direction);
	}
	return direction;

}
