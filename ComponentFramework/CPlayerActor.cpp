//#include "CPlayerActor.h"
//#include "CTransform.h"
//#include "VulkanRenderer.h"
//
//void CPlayerActor::UpdateProjectionMatrix(const float& FOVY, const float& aspectRatio, const float& nearClip, const float& farClip)
//{
//	projectionMatrix = MATH::MMath::perspective(FOVY, aspectRatio, nearClip, farClip);
//	projectionMatrix[5] *= -1.0f; // Invert Y for Vulkan
//
//}
//
//void CPlayerActor::UpdateViewMatrix()
//{
//	Ref<CTransform> transform = GetComponent<CTransform>();
//	if (transform) {
//		MATH::Vec3 pos = transform->GetPosition();
//		MATH::Quaternion rot = transform->GetRotation();
//		MATH::Matrix4 T_Inv = MATH::MMath::translate(-pos);
//		MATH::Matrix4 R_Inv = MATH::MMath::toMatrix4(MATH::QMath::conjugate(rot));
//		viewMatrix = R_Inv * T_Inv;
//		uboNeedsUpdate = true;
//	}
//}
///// <summary>
///// Creates the Cameras UBO
///// </summary>
///// <returns></returns>
//bool CPlayerActor::OnCreate()
//{
//
//	if (isCreated)
//		return true;
//	if (!render)
//		return false;
//
//	switch (render->getRendererType())
//	{
//	case RendererType::VULKAN: {
//		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
//		cameraUBO = vkrender->CreateUniformBuffers<CameraData>();
//		isCreated = true;
//		return true;
//		break;
//	}
//	default:
//
//		break;
//	}
//
//	return false;
//}
///// <summary>
///// Destroys the Cameras UBO
///// </summary>
//
//void CPlayerActor::OnDestroy()
//{
//
//	if (!isCreated)
//		return;
//	if (!render)
//		return;
//
//	switch (render->getRendererType())
//	{
//	case RendererType::VULKAN: {
//		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
//		vkrender->DestroyUBO(cameraUBO);
//		isCreated = false;
//
//		break;
//	}
//	default:
//
//		break;
//	}
//
//}
//
//
//CameraData CPlayerActor::GetCamDataUBO()
//{
//	CameraData ubo = {};
//	ubo.projectionMatrix = projectionMatrix;
//	ubo.viewMatrix = viewMatrix;
//
//	return ubo;
//}
//
///// <summary>
///// This currently updates all three UBOs at the same time
///// needs to be update to use param to only update for current frame.
///// </summary>
///// <param name="uboindex"></param>
//void CPlayerActor::UpdateUBO(uint32_t uboindex)
//{
//
//	if (!isCreated || !IsUBOOutDated())
//		return;
//	if (!render)
//		return;
//
//	switch (render->getRendererType())
//	{
//	case RendererType::VULKAN: {
//		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
//		auto data = GetCamDataUBO();
//		vkrender->UpdateUniformBuffers<CameraData>(data, cameraUBO);
//		uboNeedsUpdate = false;
//		break;
//	}
//	default:
//
//		break;
//	}
//}
//
//
