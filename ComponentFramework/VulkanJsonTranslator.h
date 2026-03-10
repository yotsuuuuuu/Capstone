#pragma once
#include "VulkanRenderer.h"
#include <string>

class VulkanJsonTranslator
{
	VkDescriptorType TranslateVulkanDes(std::string vulkan_){
		if (vulkan_ == "combined")
		{
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
		
	};
	VkShaderStageFlagBits TranslateVulkanShaderStage(std::string vulkan_)
	{
		if (vulkan_ == "fragment bit")
		{
			return VK_SHADER_STAGE_FRAGMENT_BIT;

		}


	};
};

