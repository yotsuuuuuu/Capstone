#include "CCamera.h"
#include "CTransform.h"
#include "CActor.h"
#include "VulkanRenderer.h"

bool CCamera::OnCreate()
{
	if (isCreated)
		return true;

	auto a = parent.lock();
	if (!renderer && !a)
		return false;

	auto T  = std::dynamic_pointer_cast<CActor>(a)->GetComponent<CTransform>();
	if (!T)
		return false;
	transform = T;

	UpdateProjectionMatrix(fovy, aspectRatio, nearClip, farClip);
	UpdateViewMatrix();
	switch (renderer->getRendererType())
	{
	case RendererType::VULKAN: {
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
		cameraUBO = vkrender->CreateUniformBuffers<CameraData>();
		CameraData data = {};
		data.projectionMatrix = projectionMatrix;
		data.viewMatrix = viewMatrix;
		vkrender->UpdateUniformBuffers<CameraData>(data, cameraUBO);
		isCreated = true;
		return true;
		break;
	}
	default:

		break;
	}

	return false;
}

void CCamera::OnDestroy()
{
	if (!isCreated)
		return;
	if (!renderer)
		return;

	switch (renderer->getRendererType())
	{
	case RendererType::VULKAN: {
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
		vkrender->DestroyUBO(cameraUBO);
		isCreated = false;

		break;
	}
	default:

		break;
	}
}

void CCamera::UpdateUBO(uint32_t index) {
	
	if (!isCreated )
		return;
	if (!renderer)
		return;
	
	if (auto T = transform.lock()) {
		
		if (T->needsUBOupdate) {
			UpdateViewMatrix();
		}
		else {
			return;
		}

		switch (renderer->getRendererType())
		{
		case RendererType::VULKAN: {
			VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
			CameraData data = {};
			data.projectionMatrix = projectionMatrix;
			data.viewMatrix = viewMatrix;
			// should update all buffers cause it might flicker at times	
			vkrender->UpdateUniformBuffers<CameraData>(data, cameraUBO);
			T->needsUBOupdate = false;
			break;
		}
		}
	}
	

}

void CCamera::UpdateProjectionMatrix(float fovy_, float aspectRatio_, float nearClip_, float farClip_)
{
	fovy = fovy_;
	aspectRatio = aspectRatio_;
	nearClip = nearClip_;
	farClip = farClip_;
	projectionMatrix = MMath::perspectiveVK(fovy, aspectRatio, nearClip, farClip);
}

void CCamera::UpdateViewMatrix() {
	auto T = transform.lock();
	if (T) {
		MATH::Vec3 pos = T->GetPosition();
		MATH::Quaternion rot = T->GetRotation();
		MATH::Matrix4 T_Inv = MATH::MMath::translate(-pos);
		MATH::Matrix4 R_Inv = MATH::MMath::toMatrix4(MATH::QMath::conjugate(rot));
		viewMatrix = R_Inv * T_Inv;		
	}
	
}

MATH::Vec3 CCamera::GetFrontVector() const
{
	auto T = transform.lock();
	if (!T) return MATH::Vec3(0.0f, 0.0f, -1.0f);

	MATH::Quaternion rot = T->GetRotation();
	return MATH::QMath::rotate(MATH::Vec3(0.0f, 0.0f, -1.0f), rot);
}

MATH::Vec3 CCamera::GetRightVector() const
{
	auto T = transform.lock();
	if (!T) return MATH::Vec3(1.0f, 0.0f, 0.0f);

	MATH::Quaternion rot = T->GetRotation();
	return MATH::QMath::rotate(MATH::Vec3(1.0f, 0.0f, 0.0f), rot);
}

MATH::Vec3 CCamera::GetUpVector() const
{
	auto T = transform.lock();
	if (!T) return MATH::Vec3(0.0f, 1.0f, 0.0f);

	MATH::Quaternion rot = T->GetRotation();
	return MATH::QMath::rotate(MATH::Vec3(0.0f, 1.0f, 0.0f), rot);
}


