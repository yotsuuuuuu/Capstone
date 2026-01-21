#include "CCameraActor.h"
#include "CTransform.h"
#include "VulkanRenderer.h"

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
		uboNeedsUpdate = true;
	}
}

bool CCameraActor::OnCreate()
{
	
	if (isCreated) 
		return true;
	

	return false;
}

void CCameraActor::OnDestroy()
{
	
	if(!isCreated)
		return;

}


CameraData CCameraActor::GetCamDataUBO()
{
	CameraData ubo = {};
	ubo.projectionMatrix = projectionMatrix;
	ubo.viewMatrix = viewMatrix;

	return ubo;
}

