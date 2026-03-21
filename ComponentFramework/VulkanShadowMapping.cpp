#include "VulkanRenderer.h"
#include "CGlobalLight.h"
#include "Debug.h"
//TODO: ADJUST FOR PER FRAME RESOUCES 
//TODO: PIVOT TO CASCADIN SHAOW MAPS
void VulkanRenderer::CreateGlobalShadowMappingResources(uint32_t width, uint32_t height, VkFormat format, 
	VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
	VkMemoryPropertyFlags properties, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	shadowMappingCntx.format = format;
	shadowMappingCntx.tile = tiling;
	shadowMappingCntx.useFlag = usage;
	shadowMappingCntx.aspectFlag = aspectFlags;
	shadowMappingCntx.propFlag = properties;
	shadowMappingCntx.initial = initialLayout;
	shadowMappingCntx.final = finalLayout;
	shadowMappingCntx.NumOFCascadeMaps = 3;
	VkExtent2D ex0;
	ex0.width = width;
	ex0.height = height;
	VkExtent2D ex1;
	ex1.width = width / 2;
	ex1.height = height / 2;
	VkExtent2D ex2;
	ex2.width = width / 4;
	ex2.height = height / 4;

	shadowMappingCntx.Exents = { ex0,ex1,ex2 };
	
	shadowMappingCntx.ShadowTextures2D.resize(shadowMappingCntx.NumOFCascadeMaps * getNumberOfFramesInFlight());
	shadowMappingCntx.FrameBuffers.resize(shadowMappingCntx.NumOFCascadeMaps * getNumberOfFramesInFlight());

	//VK_FILTER_LINEAR VK_FILTER_NEAREST
	// Step 2.1 create sampler for each shadow map
	CreateSampler(shadowMappingCntx.ShadowSampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,VK_TRUE,VK_FALSE);

	// Step 1 create number of swapchains images for each shadow map
	// Step 2 create image view for each shadow map
	

	for (size_t i = 0; i < getNumberOfFramesInFlight(); i++) {
		size_t base = i * shadowMappingCntx.NumOFCascadeMaps;
		Sampler2D HighRes, Medium, Low;
		createImage(shadowMappingCntx.Exents[0].width, shadowMappingCntx.Exents[0].height,
			shadowMappingCntx.format, shadowMappingCntx.tile, shadowMappingCntx.useFlag,
			shadowMappingCntx.propFlag, HighRes.image, HighRes.imageDeviceMemory);
		HighRes.imageView = createImageView(HighRes.image, shadowMappingCntx.format, shadowMappingCntx.aspectFlag);
		
		createImage(shadowMappingCntx.Exents[1].width, shadowMappingCntx.Exents[1].height,
			shadowMappingCntx.format, shadowMappingCntx.tile, shadowMappingCntx.useFlag,
			shadowMappingCntx.propFlag, Medium.image, Medium.imageDeviceMemory);
		Medium.imageView = createImageView(Medium.image, shadowMappingCntx.format, shadowMappingCntx.aspectFlag);

		createImage(shadowMappingCntx.Exents[2].width, shadowMappingCntx.Exents[2].height,
			shadowMappingCntx.format, shadowMappingCntx.tile, shadowMappingCntx.useFlag,
			shadowMappingCntx.propFlag, Low.image, Low.imageDeviceMemory);
		Low.imageView = createImageView(Low.image, shadowMappingCntx.format, shadowMappingCntx.aspectFlag);

		HighRes.sampler = shadowMappingCntx.ShadowSampler;
		Medium.sampler = shadowMappingCntx.ShadowSampler;
		Low.sampler = shadowMappingCntx.ShadowSampler;

		shadowMappingCntx.ShadowTextures2D[base + 0] = HighRes;
		shadowMappingCntx.ShadowTextures2D[base + 1] = Medium;
		shadowMappingCntx.ShadowTextures2D[base + 2] = Low;

	}
	// Step 3 create render pass for shadow mapping

	VkAttachmentDescription depthAttachment{}; 
	depthAttachment.format = shadowMappingCntx.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = shadowMappingCntx.final;

	CreateRenderPass(shadowMappingCntx.RenderPass, {}, depthAttachment);
	// step 4 create framebuffer for each image
	
	
	for (size_t i = 0; i < getNumberOfFramesInFlight(); i++) {
		size_t base = i * shadowMappingCntx.NumOFCascadeMaps;
		VkExtent2D HIGH, MED, LOW;
		HIGH = shadowMappingCntx.Exents[0];
		MED  = shadowMappingCntx.Exents[1];		
		LOW = shadowMappingCntx.Exents[2];
		CreateFrameBuffer({ shadowMappingCntx.ShadowTextures2D[base + 0].imageView }, HIGH, shadowMappingCntx.RenderPass, shadowMappingCntx.FrameBuffers[base + 0]);
		CreateFrameBuffer({ shadowMappingCntx.ShadowTextures2D[base + 1].imageView }, MED, shadowMappingCntx.RenderPass, shadowMappingCntx.FrameBuffers[base + 1]);
		CreateFrameBuffer({ shadowMappingCntx.ShadowTextures2D[base + 2].imageView }, LOW, shadowMappingCntx.RenderPass, shadowMappingCntx.FrameBuffers[base + 2]);

	}
	// Command buffers
	
	shadowMappingCntx.CMDpool = CreateCMDPool(getQueueFamilys().graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	int numerofelements = shadowMappingCntx.NumOFCascadeMaps * getNumberOfFramesInFlight();
	for (int i = 0; i < numerofelements; i++) {
		auto cmd = AllocatedCMDbuffer(shadowMappingCntx.CMDpool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		shadowMappingCntx.CMDBuffers.push_back(cmd);
	}
	
	// Ready singals
	shadowMappingCntx.ReadySignals.resize(numerofelements);

	for (int i = 0; i < shadowMappingCntx.ReadySignals.size(); i++) {
		CreateSemaphore(shadowMappingCntx.ReadySignals[i]);
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
	shadowMappingCntx.DesSetInfo.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	shadowMappingCntx.DesSetInfo.descriptorPool = CreateDescriptorPool(layout, 3);
	// step  allocate set and write to all sets in the pool
	shadowMappingCntx.DesSetInfo.descriptorSet = AllocateDescriptorSets(shadowMappingCntx.DesSetInfo.descriptorPool,
		shadowMappingCntx.DesSetInfo.descriptorSetLayout,6);
	AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::PER_FRAME_UBO, VK_SHADER_STAGE_VERTEX_BIT, 1, Glight->GetShadowUBO());
	WriteDescriptorSets(shadowMappingCntx.DesSetInfo.descriptorSet, write);

	// step 8 create pipelines for each shadow map
	shadowMappingCntx.PipelineInfo.resize(3);
	PipeLineConfig config;
	config.renderPass = shadowMappingCntx.RenderPass;
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
		config.viewPortsize = shadowMappingCntx.Exents[i];
		shadowMappingCntx.PipelineInfo[i] =
			CreateGraphicsPipeline({ shadowMappingCntx.DesSetInfo.descriptorSetLayout }, config, vertFile, fragFile);
	}
}
 

void VulkanRenderer::DestroyShadowMappingResources()
{
	
	//step 7 and 8
	for (const auto& info : shadowMappingCntx.PipelineInfo) {
		DestroyPipeline(info);
	}	
	DestroyDescriptorSet(shadowMappingCntx.DesSetInfo);
	// need to destroy in reverse order
	
	for (VkSemaphore& sema : shadowMappingCntx.ReadySignals) {
		DestroySemaphore(sema);
	}

	DestroyCommandBuffer(shadowMappingCntx.CMDBuffers,shadowMappingCntx.CMDpool);
	DestroyCommandPool(shadowMappingCntx.CMDpool);

	// step 4
	for (size_t i = 0; i < shadowMappingCntx.FrameBuffers.size(); i++) {
		DestroyFrameBuffer(shadowMappingCntx.FrameBuffers[i]);
	}

	// step 3 
	DestroyRenderPass(shadowMappingCntx.RenderPass);
	// step 2	
	// step 1
	DestroySampler(shadowMappingCntx.ShadowSampler);
	for (auto& sampler : shadowMappingCntx.ShadowTextures2D) {	
		DestroyImageView(sampler.imageView);
		DestroyImage(sampler.image, sampler.imageDeviceMemory);
	}
}