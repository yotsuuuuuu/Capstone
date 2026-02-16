#include "VulkanRenderer.h"
#include "CGlobalLight.h"
#include "Debug.h"
//TODO: ADJUST FOR PER FRAME RESOUCES 
//TODO: PIVOT TO CASCADIN SHAOW MAPS
void VulkanRenderer::CreateGlobalShadowMappingResources(uint32_t width, uint32_t height, VkFormat format, 
	VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
	VkMemoryPropertyFlags properties, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	shadowMappingInfo.Exents.height = height;
	shadowMappingInfo.Exents.width = width;
	shadowMappingInfo.format = format;
	shadowMappingInfo.tile = tiling;
	shadowMappingInfo.useFlag = usage;
	shadowMappingInfo.aspectFlag = aspectFlags;
	shadowMappingInfo.propFlag = properties;
	shadowMappingInfo.initial = initialLayout;
	shadowMappingInfo.final = finalLayout;


	shadowMappingInfo.ShadowTextures2D.resize(numSwapchains);
	shadowMappingInfo.FrameBuffers.resize(numSwapchains);
	shadowMappingInfo.WaitSemaphores.resize(MAX_FRAMES_IN_FLIGHT); // not sure ill need syncs objects
	shadowMappingInfo.SignalSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	shadowMappingInfo.InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	// Step 1 create number of swapchains images for each shadow map
	// Step 2 create image view for each shadow map
	// Step 2.1 create sampler for each shadow map
	for(  auto& sampler : shadowMappingInfo.ShadowTextures2D){
		createImage(shadowMappingInfo.Exents.width, shadowMappingInfo.Exents.height,
			shadowMappingInfo.format, shadowMappingInfo.tile, shadowMappingInfo.useFlag,
			shadowMappingInfo.propFlag, sampler.image, sampler.imageDeviceMemory);
		sampler.imageView = createImageView(sampler.image, shadowMappingInfo.format, shadowMappingInfo.aspectFlag);
		//VK_FILTER_LINEAR VK_FILTER_NEAREST
		CreateSampler(sampler.sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,VK_TRUE,VK_FALSE);
	}
	// Step 3 create render pass for shadow mapping

	VkAttachmentDescription depthAttachment{}; 
	depthAttachment.format = shadowMappingInfo.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = shadowMappingInfo.final ;

	CreateRenderPass(shadowMappingInfo.RenderPass, {}, depthAttachment);
	// step 4 create framebuffer for each image
	
	for (size_t i = 0; i < shadowMappingInfo.FrameBuffers.size(); i++) {

		CreateFrameBuffer({ shadowMappingInfo.ShadowTextures2D[i].imageView },
			shadowMappingInfo.Exents, shadowMappingInfo.RenderPass, shadowMappingInfo.FrameBuffers[i]);
	}

	// step 5 sync objects per frame in flight
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		
		CreateSemaphore(shadowMappingInfo.WaitSemaphores[i]);
		CreateSemaphore(shadowMappingInfo.SignalSemaphores[i]);
		CreateFence(shadowMappingInfo.InFlightFences[i]);
	}

	// step 6 create command buffers for each frame in flight? maybe not need maybe use the main command buffers

}

void VulkanRenderer::CreateGlobalShadowPipelineResources(std::string vertFile, std::string fragFile, std::shared_ptr<Component> globaLight)
{


	auto Glight = std::dynamic_pointer_cast<CGlobalLight>(globaLight);
	if (!Glight) {
		Debug::FatalError("NO VALID GLOBAL LIGHT COMPONENT", __FILE__, __LINE__);
		return;
	}
	std::vector<SingleDescriptorSetLayoutInfo> layout;
	std::vector<DescriptorWriteInfo> write;
	AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
	// 7.1  set layout
	shadowMappingInfo.DesSetInfo.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	shadowMappingInfo.DesSetInfo.descriptorPool = CreateDescriptorPool(layout, 1);
	// step  allocate set and write to it
	shadowMappingInfo.DesSetInfo.descriptorSet = AllocateDescriptorSets(shadowMappingInfo.DesSetInfo.descriptorPool,
		shadowMappingInfo.DesSetInfo.descriptorSetLayout);
	AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::UBO, VK_SHADER_STAGE_VERTEX_BIT, 1, Glight->GetUBO());
	WriteDescriptorSets(shadowMappingInfo.DesSetInfo.descriptorSet, write);

	// step 8 create pipeline
	PipeLineConfig config;
	config.viewPortsize = shadowMappingInfo.Exents;
	config.renderPass = shadowMappingInfo.RenderPass;
	config.cullMode = VK_CULL_MODE_FRONT_BIT; //VK_CULL_MODE_BACK_BIT VK_CULL_MODE_FRONT_BIT
	config.depthBias = VK_TRUE;
	config.depthBiasConstantFactor = 0.7f;
	config.depthBiasSlopeFactor = 2.75f;
	config.depthTestEnable = VK_TRUE;
	config.depthWriteEnable = VK_TRUE;
	config.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //VK_COMPARE_OP_LESS_OR_EQUAL VK_COMPARE_OP_LESS
	config.Color = false;

	shadowMappingInfo.PipelineInfo = CreateGraphicsPipeline({ shadowMappingInfo.DesSetInfo.descriptorSetLayout }, config, vertFile, fragFile);

}
 

void VulkanRenderer::DestroyShadowMappingResources()
{
	
	//step 7 and 8
	DestroyPipeline(shadowMappingInfo.PipelineInfo);
	DestroyDescriptorSet(shadowMappingInfo.DesSetInfo);
	// need to destroy in reverse order
	// step 5
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		DestroySemaphore(shadowMappingInfo.WaitSemaphores[i]);
		DestroySemaphore(shadowMappingInfo.SignalSemaphores[i]);
		DestroyFence(shadowMappingInfo.InFlightFences[i]);
	}

	// step 4
	for (size_t i = 0; i < shadowMappingInfo.FrameBuffers.size(); i++) {
		DestroyFrameBuffer(shadowMappingInfo.FrameBuffers[i]);
	}

	// step 3 
	DestroyRenderPass(shadowMappingInfo.RenderPass);
	// step 2	
	// step 1
	for (auto& sampler : shadowMappingInfo.ShadowTextures2D) {
		DestroySampler(sampler.sampler);
		DestroyImageView(sampler.imageView);
		DestroyImage(sampler.image, sampler.imageDeviceMemory);
	}
}