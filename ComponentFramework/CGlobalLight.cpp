#include "CGlobalLight.h"
#include "CTransform.h"
#include "CActor.h"
#include "VulkanRenderer.h"

CGlobalLight::CGlobalLight(Ref<Component> parent_, Renderer* renderer_, OrthConfig config, LightConfig data)
	: Component(parent_), renderer(renderer_), Perc({}) {
	G_data.ambient = data.ambient;
	G_data.diffused = data.diffused;
	G_data.specular = data.specular;
	G_data.projectionMatrix = MMath::orthographicVK(config.xmin,config.xmax,config.ymin,config.ymax,config.zmin,config.zmax);
	G_data.direction = Vec4(VMath::normalize(QMath::rotate(Vec3(0, 0, -1), data.orientation)),0);
	orientation = data.orientation;
	distance = data.distance;
	mode = GLMODE::ORTHO;
	Othc = config;
}

CGlobalLight::CGlobalLight(Ref<Component> parent_, Renderer* renderer_, PerspectiveConfig config, LightConfig data)
	: Component(parent_), renderer(renderer_), Othc({}) {
	G_data.ambient = data.ambient;
	G_data.diffused = data.diffused;
	G_data.specular = data.specular;
	G_data.projectionMatrix = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
	G_data.direction = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), data.orientation));
	orientation = data.orientation;
	distance = data.distance;
	mode = GLMODE::PRESPECTIVE;
	Perc = config;
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
	UpdateViewMatrix(); // NEEDS ADJSUTMENT
	
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
		MATH::Vec3 CamPos = T->GetPosition();
		MATH::Quaternion CamRot = T->GetRotation();
		MATH::Vec3 LightCenter;
		MATH::Matrix4 R_Inv = MATH::MMath::toMatrix4(MATH::QMath::conjugate(orientation));
		
		switch (mode) {
			case GLMODE::ORTHO: {
				MATH::Vec3 LightDir = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), orientation));
				MATH::Vec3 CameraFoward = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), CamRot));
				float offset = (Othc.zmax - Othc.zmin) * 0.45f;
				MATH::Vec3 ShadowCenter = CamPos + CameraFoward * offset;
				MATH::Vec3 LightPos = ShadowCenter - LightDir * offset;
				LightCenter = LightPos ;
				LightCenter = QMath::rotate(LightPos, QMath::conjugate(orientation));
				float texelSizeX = (Othc.xmax - Othc.xmin) / float(SHAWDOW_SIZE);
				float texelSizeY = (Othc.ymax - Othc.ymin) / float(SHAWDOW_SIZE);
				LightCenter.x = floor(LightCenter.x / texelSizeX) * texelSizeX;
				LightCenter.y = floor(LightCenter.y / texelSizeY) * texelSizeY;
				LightCenter = QMath::rotate(LightCenter, orientation);
				break;
			}
			case GLMODE::PRESPECTIVE: {
				MATH::Vec3 LightDir = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), orientation));
				float offset = distance;
				MATH::Vec3 LightPos = CamPos - LightDir * offset;
				LightCenter = LightPos;
				break;
			}
		}
		
		MATH::Matrix4 T_Inv = MATH::MMath::translate(-LightCenter);
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
		UpdateViewMatrix();
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);		
		vkrender->UpdateUniformBuffer<GlobalLightData>(G_data, GL_UBO[uboindex]);
		break;
	}
	}

}