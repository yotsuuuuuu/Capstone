#include "CGlobalLight.h"
#include "CTransform.h"
#include "CActor.h"
#include "VulkanRenderer.h"

CGlobalLight::CGlobalLight(Ref<Component> parent_, Renderer* renderer_, OrthConfig config, LightConfig data)
	: Component(parent_), renderer(renderer_), Perc({}) {
	G_data.ambient = data.ambient;
	G_data.diffused = data.diffused;
	G_data.specular = data.specular;
	G_data.projectionMatrix[0] = MMath::orthographicVK(config.xmin, config.xmax, config.ymin, config.ymax, config.zmin, config.zmax );	
	G_data.projectionMatrix[1] = MMath::orthographicVK(config.xmin * MedFactor, config.xmax * MedFactor, config.ymin * MedFactor, config.ymax * MedFactor, config.zmin , config.zmax * MedFactor);
	G_data.projectionMatrix[2] = MMath::orthographicVK(config.xmin * LowFactor, config.xmax * LowFactor, config.ymin * LowFactor, config.ymax * LowFactor, config.zmin , config.zmax * LowFactor);
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
	G_data.projectionMatrix[0] = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
	G_data.projectionMatrix[1] = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
	G_data.projectionMatrix[2] = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
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
	//TODO CREAT 6 UBO FOR SHADOW APSS
	// TOD CREAT 2 UBOS FOR MAIN PASS
	switch (renderer->getRendererType())
	{
	case RendererType::VULKAN: {
		VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);
		//create UBOs
		GL_UBO = vkrender->CreateUniformBuffers<GlobalLightData>();		
		ShadowsUBOs.clear();
		ShadowsUBOs.resize(6);
		for (size_t i = 0; i < ShadowsUBOs.size(); i++) {
			ShadowsUBOs[i] = vkrender->CreateUniformBuffer<CameraData>();
		}
		//Write to UBOS intial values
		for (size_t i = 0; i < vkrender->getNumberOfFramesInFlight(); i++) {
			size_t base = i * 3;

			CameraData UBO0;
			UBO0.projectionMatrix = G_data.projectionMatrix[0];
			UBO0.viewMatrix = G_data.viewMatrix[0];
			CameraData UBO1;
			UBO1.projectionMatrix = G_data.projectionMatrix[1];
			UBO1.viewMatrix = G_data.viewMatrix[1];
			CameraData UBO2;
			UBO2.projectionMatrix = G_data.projectionMatrix[2];
			UBO2.viewMatrix = G_data.viewMatrix[2];

			vkrender->UpdateUniformBuffer<CameraData>(UBO0, ShadowsUBOs[base + 0]);
			vkrender->UpdateUniformBuffer<CameraData>(UBO1, ShadowsUBOs[base + 1]);
			vkrender->UpdateUniformBuffer<CameraData>(UBO2, ShadowsUBOs[base + 2]);

		}
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
		vkrender->DestroyUBO(ShadowsUBOs);
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
	G_data.projectionMatrix[0] = MMath::orthographicVK(config.xmin, config.xmax, config.ymin, config.ymax, config.zmin, config.zmax);
	Othc = config;
	mode = GLMODE::ORTHO;
}

void CGlobalLight::SetLightProjection(PerspectiveConfig config)
{
	G_data.projectionMatrix[0] = MMath::perspectiveVK(config.fovy, config.aspectRatio, config.near, config.far);
	Perc = config;
	mode = GLMODE::PRESPECTIVE;
}
// TODO: ACCOUNT FOR MULTIPLE ORTHO VOLUMES
void CGlobalLight::UpdateViewMatrix() {
	auto T = transform.lock();
	if (T) {
		MATH::Vec3 CamPos = T->GetPosition();
		MATH::Quaternion CamRot = T->GetRotation();
		MATH::Vec3 HighLightPos;
		MATH::Vec3 MedLightPos;
		MATH::Vec3 LowhLightPos;
		MATH::Matrix4 R_Inv = MATH::MMath::toMatrix4(MATH::QMath::conjugate(orientation));
		
		switch (mode) {
			case GLMODE::ORTHO: {
				MATH::Vec3 LightDir = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), orientation));
				MATH::Vec3 CameraFoward = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), CamRot));
				
				float texelSizeX = (Othc.xmax - Othc.xmin) / float(SHAWDOW_SIZE);
				float texelSizeY = (Othc.ymax - Othc.ymin) / float(SHAWDOW_SIZE);
				float texelSize1X = ((Othc.xmax - Othc.xmin) * MedFactor) / float(SHAWDOW_SIZE / 2.0f);
				float texelSize1Y = ((Othc.ymax - Othc.ymin) * MedFactor) / float(SHAWDOW_SIZE / 2.0f);
				float texelSize2X = ((Othc.xmax - Othc.xmin) * LowFactor) / float(SHAWDOW_SIZE / 4.0f);
				float texelSize2Y = ((Othc.ymax - Othc.ymin) * LowFactor) / float(SHAWDOW_SIZE / 4.0f);
				float offset = (Othc.zmax - Othc.zmin) * 0.45f;

				//MATH::Vec3 ShadowCenter = CamPos;
				MATH::Vec3 ShadowCenter = CamPos + CameraFoward * offset;
				MATH::Vec3 LightPos0 = ShadowCenter - LightDir *  offset;
				MATH::Vec3 LightPos1 = CamPos - LightDir * offset * MedFactor;
				MATH::Vec3 LightPos2 = CamPos - LightDir * offset * LowFactor;
				HighLightPos = LightPos0;
				MedLightPos = LightPos1;
				LowhLightPos = LightPos2;

				HighLightPos = QMath::rotate(HighLightPos, QMath::conjugate(orientation));
				HighLightPos.x = floor(HighLightPos.x / texelSizeX) * texelSizeX;
				HighLightPos.y = floor(HighLightPos.y / texelSizeY) * texelSizeY;

				MedLightPos = QMath::rotate(MedLightPos, QMath::conjugate(orientation));
				MedLightPos.x = floor(MedLightPos.x / texelSize1X) * texelSize1X;
				MedLightPos.y = floor(MedLightPos.y / texelSize1Y) * texelSize1Y;
				
				LowhLightPos = QMath::rotate(LowhLightPos, QMath::conjugate(orientation));
				LowhLightPos.x = floor(LowhLightPos.x / texelSize2X) * texelSize2X;
				LowhLightPos.y = floor(LowhLightPos.y / texelSize2Y) * texelSize2Y;


				HighLightPos = QMath::rotate(HighLightPos, orientation);
				MedLightPos = QMath::rotate(MedLightPos, orientation);
				LowhLightPos = QMath::rotate(LowhLightPos, orientation);

				break;
			}
			case GLMODE::PRESPECTIVE: {
				MATH::Vec3 LightDir = VMath::normalize(QMath::rotate(Vec3(0, 0, -1), orientation));
				float offset = distance;
				MATH::Vec3 LightPos0 = CamPos - LightDir * offset;
				MATH::Vec3 LightPos1 = CamPos - LightDir * offset * MedFactor;
				MATH::Vec3 LightPos2 = CamPos - LightDir * offset * LowFactor;
				HighLightPos = LightPos0;
				MedLightPos = LightPos1;
				LowhLightPos = LightPos2;
				break;
			}
		}
		
		MATH::Matrix4 T_Inv0 = MATH::MMath::translate(-HighLightPos);
		MATH::Matrix4 T_Inv1 = MATH::MMath::translate(-MedLightPos);
		MATH::Matrix4 T_Inv2 = MATH::MMath::translate(-LowhLightPos);
		G_data.viewMatrix[0] = R_Inv * T_Inv0;
		G_data.viewMatrix[1] = R_Inv * T_Inv1;
		G_data.viewMatrix[2] = R_Inv * T_Inv2;

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

		CameraData UBO0;
		UBO0.projectionMatrix = G_data.projectionMatrix[0];
		UBO0.viewMatrix = G_data.viewMatrix[0];
		CameraData UBO1;
		UBO1.projectionMatrix = G_data.projectionMatrix[1];
		UBO1.viewMatrix = G_data.viewMatrix[1];
		CameraData UBO2;
		UBO2.projectionMatrix = G_data.projectionMatrix[2];
		UBO2.viewMatrix = G_data.viewMatrix[2];
		vkrender->UpdateUniformBuffer<CameraData>(UBO0, ShadowsUBOs[uboindex * 3 + 0]);
		vkrender->UpdateUniformBuffer<CameraData>(UBO1, ShadowsUBOs[uboindex * 3 + 1]);
		vkrender->UpdateUniformBuffer<CameraData>(UBO2, ShadowsUBOs[uboindex * 3 + 2]);

		break;
	}
	}

}