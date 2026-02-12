#include "CGlobalLight.h"
#include "CTransform.h"
#include "CActor.h"
#include "VulkanRenderer.h"

CGlobalLight::CGlobalLight(Ref<Component> parent_, Renderer* renderer_, OrthConfig config, LightConfig data)
: Component(parent_), renderer(renderer_){
	G_data.ambient = data.ambient;
	G_data.diffused = data.diffused;
	G_data.specular = data.specular;
	G_data.projectionMatrix = MMath::orthographicVK(config.xmin,config.xmax,config.ymin,config.ymax,config.zmin,config.zmax);

}

CGlobalLight::CGlobalLight(Ref<Component> parent_, Renderer* renderer_, PerspectiveConfig config, LightConfig data)
	: Component(parent_), renderer(renderer_) {
	G_data.ambient = data.ambient;
	G_data.diffused = data.diffused;
	G_data.specular = data.specular;
	G_data.projectionMatrix = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
}

bool CGlobalLight::OnCreate()
{
	if (isCreated)
		return true;

	auto a = parent.lock();
	if (!renderer && !a)
		return false;

	auto T = std::dynamic_pointer_cast<CActor>(a)->GetComponent<CTransform>();
	if (!T)
		return false;
	transform = T;
	//UpdateViewMatrix(); // NEEDS ADJSUTMENT
	switch (renderer->getRendererType())
	{
	case RendererType::VULKAN: {
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
		GL_UBO = vkrender->CreateUniformBuffers<GlobalLightData>();		
		vkrender->UpdateUniformBuffers<GlobalLightData>(G_data, GL_UBO);
		isCreated = true;
		return true;
		break;
	}
	default:

		break;
	}

	return false;
}

void CGlobalLight::OnDestroy()
{

	if (!isCreated)
		return;
	if (!renderer)
		return;

	switch (renderer->getRendererType())
	{
	case RendererType::VULKAN: {
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
		vkrender->DestroyUBO(GL_UBO);
		isCreated = false;

		break;
	}
	default:

		break;
	}
}

void CGlobalLight::SetLightData(LightConfig data)
{
	G_data.ambient = data.ambient;
	G_data.diffused = data.diffused;
	G_data.specular = data.specular;
}

void CGlobalLight::SetLightProjection(OrthConfig config)
{
	G_data.projectionMatrix = MMath::orthographicVK(config.xmin, config.xmax, config.ymin, config.ymax, config.zmin, config.zmax);
}

void CGlobalLight::SetLightProjection(PerspectiveConfig config)
{
	G_data.projectionMatrix = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
}

void CGlobalLight::UpdateViewMatrix() {
	auto T = transform.lock();
	if (T) {
		MATH::Vec3 pos = T->GetPosition();
		MATH::Quaternion rot = T->GetRotation();
		MATH::Matrix4 T_Inv = MATH::MMath::translate(-pos);
		MATH::Matrix4 R_Inv = MATH::MMath::toMatrix4(MATH::QMath::conjugate(rot));
		G_data.viewMatrix = R_Inv * T_Inv;
	}

}


void CGlobalLight::UpdateUBO(uint32_t uboindex) 
{

	if (!isCreated)
		return;
	if (!renderer)
		return;

	switch (renderer->getRendererType())
	{
	case RendererType::VULKAN: {
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);		
		vkrender->UpdateUniformBuffer<GlobalLightData>(G_data, GL_UBO[uboindex]);
		break;
	}
	}

}