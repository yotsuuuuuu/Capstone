#include "VulkanRenderer.h"



void VulkanRenderer::CreateHDRResources()
{
	//1 Create HDR RenderPass
	auto depthformat = findDepthFormat();//VK_FORMAT_D32_SFLOAT;
	auto colorformat = VK_FORMAT_R16G16B16A16_SFLOAT;
	auto numOfFramesInFLight = getNumberOfFramesInFlight();

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthformat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription ColorAttachment{};
	ColorAttachment.format = colorformat;
	ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
	// this is a exit dependacy
	VkSubpassDependency hdrDependencies{};
	hdrDependencies.srcSubpass = 0;
	hdrDependencies.dstSubpass = VK_SUBPASS_EXTERNAL;
	hdrDependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	hdrDependencies.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	hdrDependencies.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	hdrDependencies.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	CreateRenderPass(hdrInfo.hdrRenderPass, { ColorAttachment }, { depthAttachment },{ hdrDependencies });
	

	//2 Create images and depth images for HDR

	hdrInfo.hdrColor.resize(numOfFramesInFLight);

	for (auto& image : hdrInfo.hdrColor) {
		image.format = colorformat;
		image.extent = swapChainExtent;
		createImage(image.extent.width, image.extent.height, image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.image, image.memory);
		image.view = createImageView(image.image, image.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	hdrInfo.hdrDepth.resize(numOfFramesInFLight);

	for (auto& image : hdrInfo.hdrDepth) {
		image.format = depthformat;
		image.extent = swapChainExtent;
		createImage(image.extent.width, image.extent.height, image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.image, image.memory);
		image.view = createImageView(image.image, image.format, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	hdrInfo.hdrFramebuffers.resize(numOfFramesInFLight);

	for (size_t i = 0; i < hdrInfo.hdrFramebuffers.size(); i++) {

		CreateFrameBuffer({ hdrInfo.hdrColor[i].view,hdrInfo.hdrDepth[i].view }, hdrInfo.hdrColor[i].extent, hdrInfo.hdrRenderPass, hdrInfo.hdrFramebuffers[i]);
	}

	CreateSampler(hdrInfo.sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,VK_FALSE,VK_FALSE);

	// Tone Pass Descriptor set and Pipeline
	//TODO: CHANGE WHEN BLOOM TARGETS ARE DONE
	std::vector<SingleDescriptorSetLayoutInfo> layout;
	AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	hdrInfo.tonemapDescriptors.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	hdrInfo.tonemapDescriptors.descriptorPool = CreateDescriptorPool(layout, 2);

	hdrInfo.tonemapDescriptors.descriptorSet = AllocateDescriptorSets(hdrInfo.tonemapDescriptors.descriptorPool, hdrInfo.tonemapDescriptors.descriptorSetLayout, numOfFramesInFLight);

	std::vector<Sampler2D> desHDRSamplers;
	desHDRSamplers.resize(numOfFramesInFLight);
	for (size_t i = 0; i < desHDRSamplers.size(); i++) {
		desHDRSamplers[i].image = hdrInfo.hdrColor[i].image;
		desHDRSamplers[i].imageView = hdrInfo.hdrColor[i].view;
		desHDRSamplers[i].imageDeviceMemory = hdrInfo.hdrColor[i].memory;
		desHDRSamplers[i].sampler = hdrInfo.sampler;
	}	
	std::vector<DescriptorWriteInfo> write;
	AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorWriteInfo::Destype::PER_FRAME_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, desHDRSamplers);
	WriteDescriptorSets(hdrInfo.tonemapDescriptors.descriptorSet, write);

	PipeLineConfig config = GetSwapChainPipeLineConfig();	
	VkPipelineVertexInputStateCreateInfo vertexinfo{};
	vertexinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinfo.vertexBindingDescriptionCount = 0;
	vertexinfo.vertexAttributeDescriptionCount = 0;
	config.vertexinfo = vertexinfo;
	config.cullMode = VK_CULL_MODE_NONE;
	config.depthTestEnable = VK_FALSE;
	config.depthWriteEnable = VK_FALSE;
	hdrInfo.tonePassPipeline = CreateGraphicsPipeline({ hdrInfo.tonemapDescriptors.descriptorSetLayout }, config, "./shaders/TonePass.vert.spv", "./shaders/TonePass.frag.spv");
	
}

void VulkanRenderer::RecreateHDRResources()
{
	//TODO: RESIZING EVENT
}


void VulkanRenderer::DestroyHDResources()
{

	DestroyPipeline(hdrInfo.tonePassPipeline);
	DestroyDescriptorSet(hdrInfo.tonemapDescriptors);


	DestroySampler(hdrInfo.sampler);

	for (auto& frame : hdrInfo.hdrFramebuffers) {
		DestroyFrameBuffer(frame);
	}
	for (auto& image : hdrInfo.hdrColor) {
		DestroyImageView(image.view);
		DestroyImage(image.image, image.memory);
	}

	for (auto& image : hdrInfo.hdrDepth) {
		DestroyImageView(image.view);
		DestroyImage(image.image, image.memory);
	}

	DestroyRenderPass(hdrInfo.hdrRenderPass);
}