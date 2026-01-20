#include "CCameraActor.h"
#include <SDL3/SDL.h>
#include "CTransform.h"
void CCameraActor::UpdateProjectionMatrix(const float& FOVY, const float& aspectRatio, const float& nearClip, const float& farClip)
{
	projectionMatrix = MATH::MMath::perspective(FOVY, aspectRatio, nearClip, farClip);
	projectionMatrix[5] *= -1.0f; // Invert Y for Vulkan

}

void CCameraActor::UpdateViewMatrix()
{
	Ref<CTransform> transform = GetComponent<CTransform>();
	if (transform) {
		MATH::Vec3 pos = transform->GetPosition();
		MATH::Quaternion rot = transform->GetRotation();
		MATH::Matrix4 T_Inv = MATH::MMath::translate(-pos);
		MATH::Matrix4 R_Inv = MATH::MMath::toMatrix4(MATH::QMath::conjugate(rot));
		viewMatrix = R_Inv * T_Inv;
	}
}

bool CCameraActor::OnCreate()
{
	//TODO: implement camera UBO creation and initialization
	return false;
}

void CCameraActor::OnDestroy()
{
	//TODO: implement camera UBO destruction
}

