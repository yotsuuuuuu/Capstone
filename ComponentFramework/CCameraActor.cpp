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
	}
}

bool CCameraActor::OnCreate()
{
	//TODO: implement camera UBO creation and initialization
	if (isCreated) 
		return true;
	if(!renderer)
		return false;

	switch (renderer->getRendererType()) {
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		cameraUBO = vRenderer->CreateUniformBuffers<CameraData>();
		isCreated = true;
		return true;
	}
	break;
	}

	return false;
}

void CCameraActor::OnDestroy()
{
	//TODO: implement camera UBO destruction
	if(!isCreated)
		return;

	if(!renderer)
		return;

	switch (renderer->getRendererType()) {
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		vkDeviceWaitIdle(vRenderer->getDevice());
		vRenderer->DestroyUBO(cameraUBO);
		isCreated = false;
	}
	break;
	}
}

void CCameraActor::UpdateUBO()
{
	if(!isCreated)
		return;
	if (!renderer)
		return;
	UpdateViewMatrix();
	switch (renderer->getRendererType()) {
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

		vRenderer->UpdateUniformBuffer<CameraData>(GetCamDataUBO(), cameraUBO);


	}break;
	}
}

CameraData CCameraActor::GetCamDataUBO()
{
	CameraData ubo = {};
	ubo.projectionMatrix = projectionMatrix;
	ubo.viewMatrix = viewMatrix;

	return ubo;
}

