#include "VulkanRenderer.h"
#include "CGlobalLight.h"
#include "Debug.h"
//TODO: ADJUST FOR PER FRAME RESOUCES 
//TODO: PIVOT TO CASCADIN SHAOW MAPS
void VulkanRenderer::CreateGlobalShadowMappingResources(uint32_t width, uint32_t height, VkFormat format, 
	VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
	VkMemoryPropertyFlags properties, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	shadowMappingInfo.format = format;
	shadowMappingInfo.tile = tiling;
	shadowMappingInfo.useFlag = usage;
	shadowMappingInfo.aspectFlag = aspectFlags;
	shadowMappingInfo.propFlag = properties;
	shadowMappingInfo.initial = initialLayout;
	shadowMappingInfo.final = finalLayout;
	shadowMappingInfo.NumOFCascadeMaps = 3;
	VkExtent2D ex0;
	ex0.width = width;
	ex0.height = height;
	VkExtent2D ex1;
	ex1.width = width / 2;
	ex1.height = height / 2;
	VkExtent2D ex2;
	ex2.width = width / 4;
	ex2.height = height / 4;

	shadowMappingInfo.Exents = { ex0,ex1,ex2 };
	
	shadowMappingInfo.ShadowTextures2D.resize(shadowMappingInfo.NumOFCascadeMaps * getNumberOfFramesInFlight());
	shadowMappingInfo.FrameBuffers.resize(shadowMappingInfo.NumOFCascadeMaps * getNumberOfFramesInFlight());

	//VK_FILTER_LINEAR VK_FILTER_NEAREST
	// Step 2.1 create sampler for each shadow map
	CreateSampler(shadowMappingInfo.ShadowSampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,VK_TRUE,VK_FALSE);

	// Step 1 create number of swapchains images for each shadow map
	// Step 2 create image view for each shadow map
	

	for (size_t i = 0; i < getNumberOfFramesInFlight(); i++) {
		size_t base = i * shadowMappingInfo.NumOFCascadeMaps;
		Sampler2D HighRes, Medium, Low;
		createImage(shadowMappingInfo.Exents[0].width, shadowMappingInfo.Exents[0].height,
			shadowMappingInfo.format, shadowMappingInfo.tile, shadowMappingInfo.useFlag,
			shadowMappingInfo.propFlag, HighRes.image, HighRes.imageDeviceMemory);
		HighRes.imageView = createImageView(HighRes.image, shadowMappingInfo.format, shadowMappingInfo.aspectFlag);
		
		createImage(shadowMappingInfo.Exents[1].width, shadowMappingInfo.Exents[1].height,
			shadowMappingInfo.format, shadowMappingInfo.tile, shadowMappingInfo.useFlag,
			shadowMappingInfo.propFlag, Medium.image, Medium.imageDeviceMemory);
		Medium.imageView = createImageView(Medium.image, shadowMappingInfo.format, shadowMappingInfo.aspectFlag);

		createImage(shadowMappingInfo.Exents[2].width, shadowMappingInfo.Exents[2].height,
			shadowMappingInfo.format, shadowMappingInfo.tile, shadowMappingInfo.useFlag,
			shadowMappingInfo.propFlag, Low.image, Low.imageDeviceMemory);
		Low.imageView = createImageView(Low.image, shadowMappingInfo.format, shadowMappingInfo.aspectFlag);

		HighRes.sampler = shadowMappingInfo.ShadowSampler;
		Medium.sampler = shadowMappingInfo.ShadowSampler;
		Low.sampler = shadowMappingInfo.ShadowSampler;

		shadowMappingInfo.ShadowTextures2D[base + 0] = HighRes;
		shadowMappingInfo.ShadowTextures2D[base + 1] = Medium;
		shadowMappingInfo.ShadowTextures2D[base + 2] = Low;

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
	
	
	for (size_t i = 0; i < getNumberOfFramesInFlight(); i++) {
		size_t base = i * shadowMappingInfo.NumOFCascadeMaps;
		VkExtent2D HIGH, MED, LOW;
		HIGH = shadowMappingInfo.Exents[0];
		MED  = shadowMappingInfo.Exents[1];		
		LOW = shadowMappingInfo.Exents[2];
		CreateFrameBuffer({ shadowMappingInfo.ShadowTextures2D[base + 0].imageView }, HIGH, shadowMappingInfo.RenderPass, shadowMappingInfo.FrameBuffers[base + 0]);
		CreateFrameBuffer({ shadowMappingInfo.ShadowTextures2D[base + 1].imageView }, MED, shadowMappingInfo.RenderPass, shadowMappingInfo.FrameBuffers[base + 1]);
		CreateFrameBuffer({ shadowMappingInfo.ShadowTextures2D[base + 2].imageView }, LOW, shadowMappingInfo.RenderPass, shadowMappingInfo.FrameBuffers[base + 2]);

	}
	// Command buffers
	
	shadowMappingInfo.CMDpool = CreateCMDPool(getQueueFamilys().graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	int numerofelements = shadowMappingInfo.NumOFCascadeMaps * getNumberOfFramesInFlight();
	for (int i = 0; i < numerofelements; i++) {
		auto cmd = AllocatedCMDbuffer(shadowMappingInfo.CMDpool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		shadowMappingInfo.CMDBuffers.push_back(cmd);
	}
	
	// Ready singals
	shadowMappingInfo.ReadySignals.resize(numerofelements);

	for (int i = 0; i < shadowMappingInfo.ReadySignals.size(); i++) {
		CreateSemaphore(shadowMappingInfo.ReadySignals[i]);
	}

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
	// 7.1  set layout allocated 3 set pre frame
	shadowMappingInfo.DesSetInfo.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	shadowMappingInfo.DesSetInfo.descriptorPool = CreateDescriptorPool(layout, 3);
	// step  allocate set and write to all sets in the pool
	shadowMappingInfo.DesSetInfo.descriptorSet = AllocateDescriptorSets(shadowMappingInfo.DesSetInfo.descriptorPool,
		shadowMappingInfo.DesSetInfo.descriptorSetLayout,6);
	AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::PER_FRAME_UBO, VK_SHADER_STAGE_VERTEX_BIT, 1, Glight->GetShadowUBO());
	WriteDescriptorSets(shadowMappingInfo.DesSetInfo.descriptorSet, write);

	// step 8 create pipelines for each shadow map
	shadowMappingInfo.PipelineInfo.resize(3);
	PipeLineConfig config;
	config.renderPass = shadowMappingInfo.RenderPass;
	config.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT VK_CULL_MODE_FRONT_BIT VK_CULL_MODE_NONE
	config.depthBias = VK_TRUE;
	config.depthBiasConstantFactor = 0.7f;
	config.depthBiasSlopeFactor = 2.75f;
	config.depthTestEnable = VK_TRUE;
	config.depthWriteEnable = VK_TRUE;
	config.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //VK_COMPARE_OP_LESS_OR_EQUAL VK_COMPARE_OP_LESS
	config.Color = false;
	for (int i = 0; i < 3; i++)
	{
		config.viewPortsize = shadowMappingInfo.Exents[i];
		shadowMappingInfo.PipelineInfo[i] =
			CreateGraphicsPipeline({ shadowMappingInfo.DesSetInfo.descriptorSetLayout }, config, vertFile, fragFile);
	}
}
 

void VulkanRenderer::DestroyShadowMappingResources()
{
	
	//step 7 and 8
	for (const auto& info : shadowMappingInfo.PipelineInfo) {
		DestroyPipeline(info);
	}	
	DestroyDescriptorSet(shadowMappingInfo.DesSetInfo);
	// need to destroy in reverse order
	
	for (VkSemaphore& sema : shadowMappingInfo.ReadySignals) {
		DestroySemaphore(sema);
	}

	DestroyCommandBuffer(shadowMappingInfo.CMDBuffers,shadowMappingInfo.CMDpool);
	DestroyCommandPool(shadowMappingInfo.CMDpool);

	// step 4
	for (size_t i = 0; i < shadowMappingInfo.FrameBuffers.size(); i++) {
		DestroyFrameBuffer(shadowMappingInfo.FrameBuffers[i]);
	}

	// step 3 
	DestroyRenderPass(shadowMappingInfo.RenderPass);
	// step 2	
	// step 1
	DestroySampler(shadowMappingInfo.ShadowSampler);
	for (auto& sampler : shadowMappingInfo.ShadowTextures2D) {	
		DestroyImageView(sampler.imageView);
		DestroyImage(sampler.image, sampler.imageDeviceMemory);
	}
}