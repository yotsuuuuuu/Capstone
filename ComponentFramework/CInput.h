#pragma once
#include "Component.h"
#include "Vector.h"
union SDL_Event;
class CPhysics;
class CCamera;

class CInput : public Component
{
private:
	WeakRef<CPhysics> physics;
	WeakRef<CCamera> camera;

public:
	CInput(Ref<Component> parent_ = nullptr) : Component(parent_) {}

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt);

private:
	// keyboard
	bool moveForwardPressed = false;
	bool moveBackwardPressed = false;
	bool moveLeftPressed = false;
	bool moveRightPressed = false;
	bool sprintPressed = false;
	bool jumpPressed = false;

	// mouse
	float yaw = -90.0f;
	float pitch = 0.0f;
	float mouseSensitivity = 0.1f;
	bool mouseLookActive = false;

	// parameters
	float moveSpeed = 20.0f; // units per second
	float sprintMultiplier = 2.0f;
	float jumpStrength = 50.0f; // units per second
	float airControl = 0.03f;

	//// ground
	//bool isGrounded = false;
	//float groundCheckDistance = 0.1f; // distance to check for ground

public:
	// movement functions
	void HandleKeyboardInput(const SDL_Event& event);
	void HandleMouseMotion(const SDL_Event& motion);
	void HandleMouseButton(const SDL_Event& button);

	//void Update(const float dt);

	void SetMoveSpeed(float speed) { moveSpeed = speed; }
	void SetSprintMultiplier(float multiplier) { sprintMultiplier = multiplier; }
	void SetJumpStrength(float strength) { jumpStrength = strength; }
	void SetMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }

	float GetYaw() const { return yaw; }
	float GetPitch() const { return pitch; }
	bool IsMouseLookActive() const { return mouseLookActive; }

private:
	void UpdateCameraRotation();
	//void CheckGrounded();
	MATH::Vec3 CalculateMovementDirection() const;

};

