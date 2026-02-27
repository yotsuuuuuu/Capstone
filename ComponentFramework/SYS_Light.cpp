#include "SYS_Light.h"
#include "VulkanRenderer.h"
#include "CCamera.h"
#include "CActor.h"
#include "MMath.h"
SYS_Light::~SYS_Light()
{
}

bool SYS_Light::Initilize()
{
	if (cntx == nullptr || cntx->renderer == nullptr)
		return false;
	switch (cntx->renderer->getRendererType()) {
	case RendererType::VULKAN: {
		VulkanRenderer* vk = static_cast<VulkanRenderer*>(cntx->renderer);
		VkDeviceSize size = ClusterCount * sizeof(Cluster);
		ScreenClustersSSBO = vk->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,size);
		size = LightCapacity * sizeof(CLightData);
		ActiveSceneLightSSBO = vk->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
		auto cam = std::dynamic_pointer_cast<CActor>(vk->GetCurrentCamera())->GetComponent<CCamera>();
		// updating rest of data
		systemDataUBO = vk->CreateUniformBuffer<SYS_LIGHT_DATA>();
		data.inverseProjection = MMath::inverse(cam->GetProjection());
		auto planes = cam->GetzPlanes();
		data.zPlanes[0] = planes.x;
		data.zPlanes[1] = planes.y;
		int width, height;
		SDL_GetWindowSize(vk->getWindow(), &width, &height);
		data.screenDimensions[0] = static_cast<uint32_t>(width);
		data.screenDimensions[1] = static_cast<uint32_t>(height);
		vk->UpdateUniformBuffer<SYS_LIGHT_DATA>(data, systemDataUBO);
		// got the ubos and ssbos need to create the description sets and pipelines for each 
		std::vector<SingleDescriptorSetLayoutInfo> layout;
		vk->AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		vk->AddToDescriptorLayoutCollection(layout, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);

		break;
	}
		
	}
	return true;
}

void SYS_Light::ShutDonw()
{
	switch (cntx->renderer->getRendererType()) {
	case RendererType::VULKAN: {
		VulkanRenderer* vk = static_cast<VulkanRenderer*>(cntx->renderer);
		vkDeviceWaitIdle(vk->getDevice());

		vk->DestroyUBO({ ScreenClustersSSBO,ActiveSceneLightSSBO,systemDataUBO });

		break;
	}

	}
}

void SYS_Light::ComputeClusters()
{
}

void SYS_Light::ComputeLightClusters()
{
}

bool SYS_Light::RegisterLight(CLight* Light)
{
	return false;
}

bool SYS_Light::DeregisterLight(CLight* Light)
{
	return false;
}

bool SYS_Light::UpdateLightData(CLight* Light)
{
	return false;
}
