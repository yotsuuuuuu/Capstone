#include "VulkanRenderer.h"
#include "Debug.h"


void VulkanRenderer::CreateHDRResources()
{
	//1 Create HDR RenderPass
	auto depthformat = VK_FORMAT_D32_SFLOAT;
	auto colorformat = VK_FORMAT_R16G16B16A16_SFLOAT;
	auto numOfFramesInFLight = getNumberOfFramesInFlight();
	CreateSampler(hdrInfo.sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_FALSE, VK_FALSE);
	
	
	// Bloom frame buffers and render passes
	CreateBloomRenderPasses(colorformat);
	CreateBloomMipFrameBuffers(colorformat, numOfFramesInFLight);
	// TODO: Donw and up Sampling pipelines and descriptor sets

	CreateHDRRenderPass(colorformat, depthformat);
	//2 Create images and depth images for HDR
	CreateHDRFramBuffers(colorformat, depthformat, numOfFramesInFLight);

	// Tone Pass Descriptor set and Pipeline
	//TODO: CHANGE WHEN BLOOM TARGETS ARE DONE
	CreateBloomPassPipeLines(numOfFramesInFLight);
	CreateTonePassPipeLine(numOfFramesInFLight);

	
}

void VulkanRenderer::RecreateHDRResources()
{
	//TODO: RESIZING EVENT
	DestroyBloomPassPipeLines();
	DestroyTonePassPipeLine();
	DestroyBloomMipFrameBuffers();
	DestroyHDRFramBuffers();
	auto depthformat = VK_FORMAT_D32_SFLOAT;
	auto colorformat = VK_FORMAT_R16G16B16A16_SFLOAT;
	auto numOfFramesInFLight = getNumberOfFramesInFlight();
	CreateBloomMipFrameBuffers(colorformat, numOfFramesInFLight);
	CreateHDRFramBuffers(colorformat, depthformat, numOfFramesInFLight);
	CreateBloomPassPipeLines(numOfFramesInFLight);
	CreateTonePassPipeLine(numOfFramesInFLight);
}


void VulkanRenderer::DestroyHDResources()
{

	DestroyBloomPassPipeLines();
	DestroyTonePassPipeLine();


	DestroyBloomMipFrameBuffers();
	DestroyHDRFramBuffers();
	
	DestroySampler(hdrInfo.sampler);
	DestroyBloomRenderPasses();
	DestroyHDRRenderPass();
}


void VulkanRenderer::CreateHDRFramBuffers(VkFormat colorformat, VkFormat depthformat, uint32_t numOfFramesInFLight)
{
	hdrInfo.hdrColor.clear();
	hdrInfo.hdrColor.resize(numOfFramesInFLight);

	for (auto& image : hdrInfo.hdrColor) {
		image.format = colorformat;
		image.extent = swapChainExtent;
		createImage(image.extent.width, image.extent.height, image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.image, image.memory);
		image.view = createImageView(image.image, image.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
	hdrInfo.hdrDepth.clear();
	hdrInfo.hdrDepth.resize(numOfFramesInFLight);

	for (auto& image : hdrInfo.hdrDepth) {
		image.format = depthformat;
		image.extent = swapChainExtent;
		createImage(image.extent.width, image.extent.height, image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.image, image.memory);
		image.view = createImageView(image.image, image.format, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	hdrInfo.hdrFramebuffers.clear();
	hdrInfo.hdrFramebuffers.resize(numOfFramesInFLight);

	for (size_t i = 0; i < hdrInfo.hdrFramebuffers.size(); i++) {

		CreateFrameBuffer({ hdrInfo.hdrColor[i].view,hdrInfo.hdrDepth[i].view }, hdrInfo.hdrColor[i].extent, hdrInfo.hdrRenderPass, hdrInfo.hdrFramebuffers[i]);
	}
}

void VulkanRenderer::CreateHDRRenderPass(VkFormat colorformat, VkFormat depthformat)
{
	
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
	CreateRenderPass(hdrInfo.hdrRenderPass, { ColorAttachment }, { depthAttachment }, { hdrDependencies });
}

void VulkanRenderer::CreateTonePassPipeLine(uint32_t numOfFramesInFLight)
{

	std::vector<SingleDescriptorSetLayoutInfo> layout;
	AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	AddToDescriptorLayoutCollection(layout, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	hdrInfo.tonemapDescriptors.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	hdrInfo.tonemapDescriptors.descriptorPool = CreateDescriptorPool(layout, 2);

	PipeLineConfig config = GetSwapChainPipeLineConfig();
	VkPipelineVertexInputStateCreateInfo vertexinfo{};
	vertexinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinfo.vertexBindingDescriptionCount = 0;
	vertexinfo.vertexAttributeDescriptionCount = 0;
	config.vertexinfo = vertexinfo;
	config.cullMode = VK_CULL_MODE_NONE;
	config.depthTestEnable = VK_FALSE;
	config.depthWriteEnable = VK_FALSE;

	/*Debug::Trace("HDR TONE PIPELINE", __FILE__, __LINE__);
	std::string viewportstring = "Viewport size: {" + std::to_string(config.viewPortsize.width) + "x" + std::to_string(config.viewPortsize.height) + "}";
	Debug::Trace(viewportstring, __FILE__, __LINE__);*/
	hdrInfo.tonePassPipeline = CreateGraphicsPipeline({ hdrInfo.tonemapDescriptors.descriptorSetLayout }, config, "./shaders/TonePass.vert.spv", "./shaders/TonePass.frag.spv");




	hdrInfo.tonemapDescriptors.descriptorSet = AllocateDescriptorSets(hdrInfo.tonemapDescriptors.descriptorPool, hdrInfo.tonemapDescriptors.descriptorSetLayout, numOfFramesInFLight);

	std::vector<Sampler2D> desHDRSamplers;
	desHDRSamplers.resize(numOfFramesInFLight);
	for (size_t i = 0; i < desHDRSamplers.size(); i++) {
		desHDRSamplers[i]= hdrInfo.hdrColor[i].MakeSampler(hdrInfo.sampler);
	
	}

	std::vector<Sampler2D> bloomSamples;
	bloomSamples.resize(numOfFramesInFLight);
	for (size_t i = 0; i < bloomSamples.size(); i++) {
		size_t base = hdrInfo.bloomMipLevels ;
		bloomSamples[i] = hdrInfo.bloomMips[base * i].MakeSampler(hdrInfo.sampler);

	}
	std::vector<DescriptorWriteInfo> write;
	AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorWriteInfo::Destype::PER_FRAME_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, desHDRSamplers);
	AddToDescrisptorLayoutWrite(write, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorWriteInfo::Destype::PER_FRAME_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, bloomSamples);
	WriteDescriptorSets(hdrInfo.tonemapDescriptors.descriptorSet, write);



	
}

void VulkanRenderer::CreateBloomPassPipeLines(uint32_t numOfFramesInFLight)
{

	std::vector<SingleDescriptorSetLayoutInfo> layout;
	AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	hdrInfo.bloomDescriptors.descriptorSetLayout = CreateDescriptorSetLayout(layout);
	hdrInfo.bloomDescriptors.descriptorPool = CreateDescriptorPool(layout, 9);
	
	VkPipelineVertexInputStateCreateInfo vertexinfo{};
	vertexinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinfo.vertexBindingDescriptionCount = 0;
	vertexinfo.vertexAttributeDescriptionCount = 0;
	
	PipeLineConfig config{};	
	config.cullMode = VK_CULL_MODE_NONE;
	config.vertexinfo = vertexinfo;
	config.depthBias = VK_FALSE;
	config.depthBiasClamp = 0.0f;
	config.depthBiasConstantFactor = 0.0f;
	config.depthBiasSlopeFactor = 0.0f;
	config.depthTestEnable = VK_FALSE;
	config.depthWriteEnable = VK_FALSE;
	config.depthCompareOp = VK_COMPARE_OP_LESS;
	config.polygonMode = VK_POLYGON_MODE_FILL;
	config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	config.dynamicViewport = true;
	config.blendMode = PipeLineConfig::BlendMode::OPAQUE;
	config.Color = true;
	config.renderPass = hdrInfo.bloomDonwPass; 
	config.viewPortsize = swapChainExtent; 

	/*Debug::Trace("HDR BLOOM PIPELINE", __FILE__, __LINE__);
	std::string viewportstring = "Viewport size: {" + std::to_string(config.viewPortsize.width) + "x" + std::to_string(config.viewPortsize.height) + "}";
	Debug::Trace(viewportstring, __FILE__, __LINE__);*/
	hdrInfo.thresholdPipeline = CreateGraphicsPipeline({ hdrInfo.bloomDescriptors.descriptorSetLayout }, config, "./shaders/ThresholdSample.vert.spv", "./shaders/ThresholdSample.frag.spv");
	/* viewportstring = "Viewport size: {" + std::to_string(config.viewPortsize.width) + "x" + std::to_string(config.viewPortsize.height) + "}";
	Debug::Trace(viewportstring, __FILE__, __LINE__);*/
	hdrInfo.donwSamplePipeline = CreateGraphicsPipeline({ hdrInfo.bloomDescriptors.descriptorSetLayout }, config, "./shaders/Sampling.vert.spv", "./shaders/DownSampling.frag.spv");

	config.renderPass = hdrInfo.bloomUpPass;
	config.blendMode = PipeLineConfig::BlendMode::ADDITIVE;
	 /*viewportstring = "Viewport size: {" + std::to_string(config.viewPortsize.width) + "x" + std::to_string(config.viewPortsize.height) + "}";
	Debug::Trace(viewportstring, __FILE__, __LINE__);*/
	hdrInfo.upSamplePipeline = CreateGraphicsPipeline({ hdrInfo.bloomDescriptors.descriptorSetLayout }, config, "./shaders/Sampling.vert.spv", "./shaders/UpSampling.frag.spv");

	int miplvls = static_cast<int>(hdrInfo.bloomMipLevels);
	int numberofframes = static_cast<int>(numOfFramesInFLight);
	int numberofRenderpases = 2;
	int numofdescriptorssets = ((miplvls * numberofRenderpases) - 1);
	int numofdescriptorssetsperframe = numofdescriptorssets  * numberofframes; // should be 18
	
	hdrInfo.bloomDescriptors.descriptorSet = AllocateDescriptorSets(hdrInfo.bloomDescriptors.descriptorPool, 
											hdrInfo.bloomDescriptors.descriptorSetLayout, numofdescriptorssetsperframe);

	
	std::vector<Sampler2D> samplers;
	samplers.resize(numofdescriptorssetsperframe);
	for (size_t i = 0; i < numberofframes; i++) {
		// threshold sample
		size_t frameBase = numofdescriptorssets * i;
		samplers[frameBase] = hdrInfo.hdrColor[i].MakeSampler(hdrInfo.sampler);
		// donw sample
		for (size_t j = 0; j < hdrInfo.bloomMipLevels - 1; j++) {
			size_t sourceMip = i * hdrInfo.bloomMipLevels + j;			
			samplers[frameBase + 1 + j] = hdrInfo.bloomMips[sourceMip].MakeSampler(hdrInfo.sampler);
		}
		// up samples
		for (size_t k = 0; k < hdrInfo.bloomMipLevels - 1; k++) {
			size_t sourceMip = i * hdrInfo.bloomMipLevels + (hdrInfo.bloomMipLevels - 1 - k);				
			samplers[frameBase + hdrInfo.bloomMipLevels + k] = hdrInfo.bloomMips[sourceMip].MakeSampler(hdrInfo.sampler);
		}
	}

	std::vector< DescriptorWriteInfo> write;
	AddToDescrisptorLayoutWrite(write,0,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorWriteInfo::Destype::PER_FRAME_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1,samplers);
	WriteDescriptorSets(hdrInfo.bloomDescriptors.descriptorSet, write);

}

void VulkanRenderer::CreateBloomRenderPasses(VkFormat colorformat)
{

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	{// donw bloom render pass
		VkAttachmentDescription ColorAttachment{};
		ColorAttachment.format = colorformat;
		ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		CreateRenderPass(hdrInfo.bloomDonwPass, { ColorAttachment }, std::nullopt, { dependency });

	}
	{// up bloom render pass
		VkAttachmentDescription ColorAttachment{};
		ColorAttachment.format = colorformat;
		ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		CreateRenderPass(hdrInfo.bloomUpPass, { ColorAttachment }, std::nullopt, { dependency });

	}
}

void VulkanRenderer::CreateBloomMipFrameBuffers(VkFormat colorformat, uint32_t numOfFramesInFLight)
{
	// create the bloom image and image views
	hdrInfo.bloomMips.resize(numOfFramesInFLight * hdrInfo.bloomMipLevels);
	//needs to done in steps as the render targets shared some handels

	// frist extends
	std::vector<VkExtent2D> mipExtents(hdrInfo.bloomMipLevels);
	VkExtent2D mipExtent = swapChainExtent;
	for (size_t j = 0; j < hdrInfo.bloomMipLevels; j++) {
		mipExtents[j] = mipExtent;
		mipExtent.width = std::max(1u, mipExtent.width / 2);
		mipExtent.height = std::max(1u, mipExtent.height / 2);
	}
	std::vector< VkImage> images(numOfFramesInFLight);
	std::vector< VkDeviceMemory> imagesMems(numOfFramesInFLight);
	for (size_t i = 0; i < numOfFramesInFLight; i++) {
		CreateImage(swapChainExtent.width, swapChainExtent.height, colorformat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, hdrInfo.bloomMipLevels, images[i], imagesMems[i]);
	}

	// filling each render targets information
	for (size_t i = 0; i < numOfFramesInFLight; i++) {
		for (size_t j = 0; j < hdrInfo.bloomMipLevels; j++) {
			auto index = i * static_cast<uint32_t>(hdrInfo.bloomMipLevels) + j;
			hdrInfo.bloomMips[index].image = images[i];
			hdrInfo.bloomMips[index].memory = imagesMems[i];
			hdrInfo.bloomMips[index].extent = mipExtents[j];
			hdrInfo.bloomMips[index].format = colorformat;
			CreateImageView(images[i], colorformat, static_cast<uint32_t>(j), VK_IMAGE_ASPECT_COLOR_BIT, hdrInfo.bloomMips[index].view);

		}
	}

	// need 20 buffer frames = number of bloom mips 5 X number of renderpasses 2 X number of frames in flight 2
	hdrInfo.bloomDownFramebuffers.resize(numOfFramesInFLight * hdrInfo.bloomMipLevels);
	hdrInfo.bloomUpFramebuffers.resize(numOfFramesInFLight * hdrInfo.bloomMipLevels);
	
	for (size_t i = 0; i < numOfFramesInFLight; i++) {
		for (size_t j = 0; j < hdrInfo.bloomMipLevels; j++) {
			uint32_t  index = static_cast<uint32_t>(i * hdrInfo.bloomMipLevels + j);
			auto& renderTarget = hdrInfo.bloomMips[index];
			CreateFrameBuffer({ renderTarget.view }, renderTarget.extent, hdrInfo.bloomDonwPass, hdrInfo.bloomDownFramebuffers[index]);
			CreateFrameBuffer({ renderTarget.view }, renderTarget.extent, hdrInfo.bloomUpPass, hdrInfo.bloomUpFramebuffers[index]);
		}
	}
}


void VulkanRenderer::DestroyHDRFramBuffers()
{
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

}

void VulkanRenderer::DestroyHDRRenderPass()
{
	DestroyRenderPass(hdrInfo.hdrRenderPass);
}

void VulkanRenderer::DestroyTonePassPipeLine()
{

	DestroyPipeline(hdrInfo.tonePassPipeline);
	DestroyDescriptorSet(hdrInfo.tonemapDescriptors);
}

void VulkanRenderer::DestroyBloomPassPipeLines()
{
	DestroyPipeline(hdrInfo.thresholdPipeline);
	DestroyPipeline(hdrInfo.donwSamplePipeline);
	DestroyPipeline(hdrInfo.upSamplePipeline);
	DestroyDescriptorSet(hdrInfo.bloomDescriptors);
}


void VulkanRenderer::DestroyBloomMipFrameBuffers()
{

	for (auto& frame : hdrInfo.bloomDownFramebuffers) {
		DestroyFrameBuffer(frame);
	}
	for (auto& frame : hdrInfo.bloomUpFramebuffers) {
		DestroyFrameBuffer(frame);
	}

	for (auto& imageview : hdrInfo.bloomMips) {
		DestroyImageView(imageview.view);
	}

	
	for (size_t i = 0; i < getNumberOfFramesInFlight(); i++) {
		auto index = i * static_cast<uint32_t>(hdrInfo.bloomMipLevels) + 0;
		VkImage im = hdrInfo.bloomMips[index].image;
		VkDeviceMemory mem = hdrInfo.bloomMips[index].memory;
		DestroyImage(im, mem);
	}	
}

void VulkanRenderer::DestroyBloomRenderPasses()
{

	DestroyRenderPass(hdrInfo.bloomDonwPass);
	DestroyRenderPass(hdrInfo.bloomUpPass);
}
