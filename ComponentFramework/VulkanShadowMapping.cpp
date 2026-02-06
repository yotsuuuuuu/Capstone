#include "VulkanRenderer.h"


void VulkanRenderer::CreateGlobalShadowMappingResources(uint32_t width, uint32_t height, VkFormat format, 
	VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
	VkMemoryPropertyFlags properties, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	shadowMappingInfo.Diemensions.height = height;
	shadowMappingInfo.Diemensions.width = width;
	shadowMappingInfo.format = format;
	shadowMappingInfo.tile = tiling;
	shadowMappingInfo.useFlag = usage;
	shadowMappingInfo.aspectFlag = aspectFlags;
	shadowMappingInfo.propFlag = properties;
	shadowMappingInfo.initial = initialLayout;
	shadowMappingInfo.final = finalLayout;


	shadowMappingInfo.ShadowTextures2D.resize(numSwapchains);
	shadowMappingInfo.FrameBuffers.resize(numSwapchains);
	shadowMappingInfo.WaitSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	shadowMappingInfo.SignalSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	shadowMappingInfo.InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	// Step 1 create number of swapchains images for each shadow map
	// Step 2 create image view for each shadow map
	// Step 2.1 create sampler for each shadow map
	for(  auto& sampler : shadowMappingInfo.ShadowTextures2D){
		createImage(shadowMappingInfo.Diemensions.width, shadowMappingInfo.Diemensions.height,
			shadowMappingInfo.format, shadowMappingInfo.tile, shadowMappingInfo.useFlag,
			shadowMappingInfo.propFlag, sampler.image, sampler.imageDeviceMemory);
		sampler.imageView = createImageView(sampler.image, shadowMappingInfo.format, shadowMappingInfo.aspectFlag);
		// todo need a create sampler function
		CreateSampler(sampler.sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
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
			shadowMappingInfo.Diemensions, shadowMappingInfo.RenderPass, shadowMappingInfo.FrameBuffers[i]);
	}

	// step 5 sync objects per frame in flight
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		
		CreateSemaphore(shadowMappingInfo.WaitSemaphores[i]);
		CreateSemaphore(shadowMappingInfo.SignalSemaphores[i]);
		CreateFence(shadowMappingInfo.InFlightFences[i]);
	}

	// step 6 create command buffers for each frame in flight? maybe not need maybe use the main command buffers

}

void VulkanRenderer::CreateGlobalShadowPipelineResources(std::string vertFile, std::string fragFile, WeakRef<Component> globaLight)
{
	// i dont knwo if we need shader component or just make the pipepine with in this 
	// function
	// if  used the shader ref it can only make a graphics pipeline with the 
	// dummy variables to get it working
	vertFile = "./shaders/GlobalLightvert.spv";
	fragFile = "./shaders/GlobalLightfrag.spv";
	// step 7 create descriptor set
	
	// idea for the light ubo
	// ubo have to be specially made 
	// compoente light ubo have a parented to the actor
	// oncreate acess parent gather data form other components
	// construct should be made tkaing data that cant be gather form other componets
	//  so like light color , type of light, intesity, raidus on influence
	std::vector<BufferMemory> ubo;
	std::vector<SingleDescriptorSetLayoutInfo> layout;
	std::vector<DescriptorWriteInfo> write;
	AddToDescrisptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
	// 7.1 layout
	shadowMappingInfo.DesSetInfo.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	shadowMappingInfo.DesSetInfo.descriptorPool = CreateDescriptorPool(layout, 1);
	// step  allocate set and write to it
	shadowMappingInfo.DesSetInfo.descriptorSet = AllocateDescriptorSets(shadowMappingInfo.DesSetInfo.descriptorPool,
		shadowMappingInfo.DesSetInfo.descriptorSetLayout);
	AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, ubo);
	WriteDescriptorSets(shadowMappingInfo.DesSetInfo.descriptorSet, write);

	// step 8 create pipeline
	PipeLineConfig config;
	config.viewPortsize = shadowMappingInfo.Diemensions;
	config.renderPass = shadowMappingInfo.RenderPass;
	config.cullMode = VK_CULL_MODE_FRONT_BIT;
	config.depthBias = VK_TRUE;
	config.depthBiasConstantFactor = 0.0005f;
	config.depthBiasSlopeFactor = 1.5f;
	config.depthTestEnable = VK_TRUE;
	config.depthWriteEnable = VK_TRUE;
	config.depthCompareOp = VK_COMPARE_OP_LESS;
	config.Color = false;

	shadowMappingInfo.PipelineInfo = CreateGraphicsPipeline({ shadowMappingInfo.DesSetInfo.descriptorSetLayout }, config, vertFile, fragFile);

}
 

void VulkanRenderer::DestroyShadowMappingResources()
{
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