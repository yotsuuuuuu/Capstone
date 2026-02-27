#include "SYS_Light.h"
#include "VulkanRenderer.h"

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
		systemDataUBO = vk->CreateUniformBuffer<SYS_LIGHT_DATA>();

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
