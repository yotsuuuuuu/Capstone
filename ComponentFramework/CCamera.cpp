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
		

		switch (renderer->getRendererType())
		{
		case RendererType::VULKAN: {
			VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
			CameraData data = {};
			data.projectionMatrix = projectionMatrix;
			data.viewMatrix = viewMatrix;
			// should update all buffers cause it might flicker at times	
			//vkrender->UpdateUniformBuffers<CameraData>(data, cameraUBO);
			vkrender->UpdateUniformBuffer<CameraData>(data, cameraUBO[index]);
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

std::vector<float> CCamera::GetProjMatrixValues()
{
	std::vector<float> data(4);
	data[0] = fovy;
	data[1] = aspectRatio;
	data[2] = nearClip;
	data[3] = farClip;
	return data;
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

std::vector<MATHEX::Plane> CCamera::GenerateFrustumPlane()
{
	std::vector<MATHEX::Plane> fusturm;
	Matrix4 proj = GetProjectionMatrix() * GetViewMatrix();

	MATHEX::Plane left, right, top, bottom, near, far;
	left.x = proj[3] + proj[0];
	left.y = proj[7] + proj[4];
	left.z = proj[11] + proj[8];
	left.d = (proj[15] + proj[12]);

	right.x = proj[3] - proj[0];
	right.y = proj[7] - proj[4];
	right.z = proj[11] - proj[8];
	right.d = (proj[15] - proj[12]);

	bottom.x = proj[3] + proj[1];
	bottom.y = proj[7] + proj[5];
	bottom.z = proj[11] + proj[9];
	bottom.d = (proj[15] + proj[13]);

	top.x = proj[3] - proj[1];
	top.y = proj[7] - proj[5];
	top.z = proj[11] - proj[9];
	top.d = (proj[15] - proj[13]);

	near.x = proj[3] + proj[2];
	near.y = proj[7] + proj[6];
	near.z = proj[11] + proj[10];
	near.d = proj[15] + proj[14];

	far.x = proj[3] - proj[2];
	far.y = proj[7] - proj[6];
	far.z = proj[11] - proj[10];
	far.d = (proj[15] - proj[14]);

	// Normalizaiont matters if we care for the actual distance
	// when we do the dot product.
	// if we are just checking below or above 0 then 
	// no need to normalize.
	left = MATHEX::PMath::normalize(left);
	right = MATHEX::PMath::normalize(right);
	bottom = MATHEX::PMath::normalize(bottom);
	top = MATHEX::PMath::normalize(top);
	near = MATHEX::PMath::normalize(near);
	far = MATHEX::PMath::normalize(far);

	fusturm.push_back(left);
	fusturm.push_back(right);
	fusturm.push_back(bottom);
	fusturm.push_back(top);
	fusturm.push_back(near);
	fusturm.push_back(far);
	return fusturm;
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


