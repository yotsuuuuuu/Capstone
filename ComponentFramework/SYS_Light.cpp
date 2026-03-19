#include "SYS_Light.h"
#include "VulkanRenderer.h"
#include "AssetManager.h"	
#include "CCamera.h"
#include "CActor.h"
#include "MMath.h"

namespace LIGHT_SYS_CONST {
	constexpr uint32_t  LOCAL_SIZE = 128;
	constexpr uint32_t  GRID_X = 12;
	constexpr uint32_t  GRID_Y = 12;
	constexpr uint32_t  GRID_Z = 24;
	//constexpr uint32_t  GRID_X = 40;
	//constexpr uint32_t  GRID_Y = 23;
	//constexpr uint32_t  GRID_Z = 24;
	constexpr uint32_t  TOTAL_CLUSTERS = GRID_X * GRID_Y * GRID_Z;
	constexpr uint32_t  GROUP_COUNT = (TOTAL_CLUSTERS + LOCAL_SIZE - 1) / LOCAL_SIZE;
}

SYS_Light::SYS_Light(EngineContext* cntx_, uint32_t LightCapacity_)
	: cntx(cntx_), LightCapacity(LightCapacity_), LightCount(0), ScreenClustersSSBO({}),
	systemDataUBO({}), ActiveSceneLightSSBO({}), CC_Pipelineinfo({}), CL_Pipelineinfo({}),
	ComputePool(VK_NULL_HANDLE),SignalSema(VK_NULL_HANDLE),
	Fence(VK_NULL_HANDLE), mapppedLightSSBO(nullptr)
{
	// part data has been filled in rest has to be filled on the Init
	ClusterCount = LIGHT_SYS_CONST::TOTAL_CLUSTERS;
	data.gridSize[0] =  LIGHT_SYS_CONST::GRID_X;
	data.gridSize[1] =  LIGHT_SYS_CONST::GRID_Y;
	data.gridSize[2] =  LIGHT_SYS_CONST::GRID_Z;
}
SYS_Light::~SYS_Light()
{
}

bool SYS_Light::Initilize()
{
	if (cntx == nullptr || cntx->renderer == nullptr)
		return false;
	if (isInit == true)
		return true;


	switch (cntx->renderer->getRendererType()) {
	case RendererType::VULKAN: {
		VulkanRenderer* vk = static_cast<VulkanRenderer*>(cntx->renderer);
		VkDeviceSize size = ClusterCount * sizeof(Cluster);
		//Allocation of SSBO's
		ScreenClustersSSBO.bufferMemoryLength = size;
		ScreenClustersSSBO = vk->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,size); //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		
		
		size = LightCapacity * sizeof(CLightData);
		ActiveSceneLightSSBO.bufferMemoryLength = size;
		ActiveSceneLightSSBO = vk->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
		if (vkMapMemory(vk->getDevice(), ActiveSceneLightSSBO.bufferMemoryID, 0, ActiveSceneLightSSBO.bufferMemoryLength, 0, &mapppedLightSSBO) != VK_SUCCESS)
			return false;
		
		camera = cntx->assetManager->GetCamera();
		auto tempCam = camera.lock();
		if (!tempCam) 
			return false;

		auto cam = std::dynamic_pointer_cast<CActor>(tempCam)->GetComponent<CCamera>();
	
		// updating rest of data
		systemDataUBO = vk->CreateUniformBuffer<SYS_LIGHT_DATA>();
		// next four  var are dependent on the Camera
		// if the cameras projection matrix changes (most likly due to screen size change)
		// these values should get updated.
		data.inverseProjection = MMath::inverse(cam->GetProjection());
		auto planes = cam->GetzPlanes();
		data.zPlanes[0] = planes.x;
		data.zPlanes[1] = planes.y;
		int width, height;
		SDL_GetWindowSize(vk->getWindow(), &width, &height); // depends on the window size  needs to update on screen size change
		data.screenDimensions[0] = static_cast<uint32_t>(width);
		data.screenDimensions[1] = static_cast<uint32_t>(height);
		data.lightCount = LightCount; // should start at 0 
		data.clusterCount = ClusterCount; // fixed
		vk->UpdateUniformBuffer<SYS_LIGHT_DATA>(data, systemDataUBO);
		systemDataUBOOutOfDate = false;
		// Create the description sets and pipelines for each 
		// Cluster AABB compute and Light Cull
		std::vector<SingleDescriptorSetLayoutInfo> layout;
		vk->AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		vk->AddToDescriptorLayoutCollection(layout, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		CC_DescriptorSetInfo.descriptorSetLayout = vk->CreateDescriptorSetLayout(layout);
		CC_DescriptorSetInfo.descriptorPool = vk->CreateDescriptorPool(layout, 1);
		CC_DescriptorSetInfo.descriptorSet = vk->AllocateDescriptorSets(CC_DescriptorSetInfo.descriptorPool, CC_DescriptorSetInfo.descriptorSetLayout, 1);
		std::vector<DescriptorWriteInfo> write;
		vk->AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DescriptorWriteInfo::Destype::STATIC_SSBO, VK_SHADER_STAGE_COMPUTE_BIT, 1, { ScreenClustersSSBO });
		vk->AddToDescrisptorLayoutWrite(write, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::STATIC_UBO, VK_SHADER_STAGE_COMPUTE_BIT, 1, { systemDataUBO });
		vk->WriteDescriptorSets(CC_DescriptorSetInfo.descriptorSet, write);
		CC_Pipelineinfo = vk->CreateComputePipeline({ CC_DescriptorSetInfo.descriptorSetLayout }, "shaders/ClusterCompute.comp.spv");
		// Light Culling pass
		layout.clear();
		write.clear();

		
		vk->AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		vk->AddToDescriptorLayoutCollection(layout, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		vk->AddToDescriptorLayoutCollection(layout, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		vk->AddToDescriptorLayoutCollection(layout, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		CL_DescriptorSetInfo.descriptorSetLayout = vk->CreateDescriptorSetLayout(layout);
		CL_DescriptorSetInfo.descriptorPool = vk->CreateDescriptorPool(layout, 1);
		CL_DescriptorSetInfo.descriptorSet = vk->AllocateDescriptorSets(CL_DescriptorSetInfo.descriptorPool, CL_DescriptorSetInfo.descriptorSetLayout, 2);
		
		vk->AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::PER_FRAME_UBO, VK_SHADER_STAGE_COMPUTE_BIT, 1, cam->GetCameraUBO());
		vk->AddToDescrisptorLayoutWrite(write, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::STATIC_UBO, VK_SHADER_STAGE_COMPUTE_BIT, 1, { systemDataUBO });
		vk->AddToDescrisptorLayoutWrite(write, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DescriptorWriteInfo::Destype::STATIC_SSBO, VK_SHADER_STAGE_COMPUTE_BIT, 1, { ScreenClustersSSBO });
		vk->AddToDescrisptorLayoutWrite(write, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DescriptorWriteInfo::Destype::STATIC_SSBO, VK_SHADER_STAGE_COMPUTE_BIT, 1, { ActiveSceneLightSSBO });

		vk->WriteDescriptorSets(CL_DescriptorSetInfo.descriptorSet, write);
		//TODO: SET FILE PATH
		CL_Pipelineinfo = vk->CreateComputePipeline({ CL_DescriptorSetInfo.descriptorSetLayout }, "shaders/LightCullCompute.comp.spv");
		// Command pool and buffer.
		ComputePool = vk->CreateCMDPool(vk->getQueueFamilys().computeFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		ComputeCmd.push_back( vk->AllocatedCMDbuffer(ComputePool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		ComputeCmd.push_back(vk->AllocatedCMDbuffer(ComputePool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		// sync objects
		vk->CreateSemaphore(SignalSema);
		vk->CreateFence(Fence);


		break;
	}
		
	}

	isInit = true;
	return true;
}

void SYS_Light::ShutDonw()
{
	if (isInit == false)
		return;

	switch (cntx->renderer->getRendererType()) {
	case RendererType::VULKAN: {
		VulkanRenderer* vk = static_cast<VulkanRenderer*>(cntx->renderer);
		vkDeviceWaitIdle(vk->getDevice());

		vkUnmapMemory(vk->getDevice(), ActiveSceneLightSSBO.bufferMemoryID);
		vk->DestroySemaphore(SignalSema);
		vk->DestroyFence(Fence);

		
		vk->DestroyCommandBuffer(ComputeCmd, ComputePool);
		vk->DestroyCommandPool(ComputePool);

		vk->DestroyDescriptorSet(CC_DescriptorSetInfo);
		vk->DestroyPipeline(CC_Pipelineinfo);

		vk->DestroyDescriptorSet(CL_DescriptorSetInfo);
		vk->DestroyPipeline(CL_Pipelineinfo);

		vk->DestroyUBO({ ScreenClustersSSBO,ActiveSceneLightSSBO,systemDataUBO });

		break;
	}

	}

	isInit = false;
}

void SYS_Light::ComputeClusters()
{
	switch (cntx->renderer->getRendererType()) {
	case RendererType::VULKAN: {
		VulkanRenderer* vk = static_cast<VulkanRenderer*>(cntx->renderer);
		auto device = vk->getDevice();
		vkWaitForFences(device, 1, &Fence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &Fence);
		if (systemDataUBOOutOfDate) {
			vk->UpdateUniformBuffer<SYS_LIGHT_DATA>(data, systemDataUBO);
			systemDataUBOOutOfDate = false;
		}
		vk->CMDBeginRecord(ComputeCmd.front());
		vk->CMDRecordBindPipeline(ComputeCmd.front(), CC_Pipelineinfo.pipeline, VK_PIPELINE_BIND_POINT_COMPUTE);
		vk->CMDRecordDescriptorSet(ComputeCmd.front(), CC_Pipelineinfo.pipelineLayout, VK_PIPELINE_BIND_POINT_COMPUTE, &CC_DescriptorSetInfo.descriptorSet.front());
		//  now distpatch
		vk->CMDRecordDistpatch(ComputeCmd.front(), LIGHT_SYS_CONST::GRID_X, LIGHT_SYS_CONST::GRID_Y, LIGHT_SYS_CONST::GRID_Z);
		// end record
		vk->CMDEndRecord(ComputeCmd.front());
		// submit
		vk->CMDSubmitComputeQueue(&ComputeCmd.front(), 1, Fence);
		//vkWaitForFences(device, 1, &Fence, VK_TRUE, UINT64_MAX);
		////// Temporary debug readback
		//	void* mappedData;
		//vkMapMemory(device, ScreenClustersSSBO.bufferMemoryID, 0, sizeof(Cluster) * 5, 0, &mappedData);
		//Cluster* clusters = (Cluster*)mappedData;
		//for (int i = 0; i < 435; i++)
		//{
		//	if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4) {
		//		printf("Cluster %d min: %.5f %.5f %.5f | max: %.5f %.5f %.5f\n", i,
		//			clusters[i].minPoint.x, clusters[i].minPoint.y, clusters[i].minPoint.z,
		//			clusters[i].maxPoint.x, clusters[i].maxPoint.y, clusters[i].maxPoint.z);
		//	}
		//}
		//vkUnmapMemory(device, ScreenClustersSSBO.bufferMemoryID);
	   break;
	}
	}
}



void SYS_Light::ComputeLightClusters(uint32_t frameIndex)
{
	auto cmd = ComputeCmd[frameIndex];
	switch (cntx->renderer->getRendererType()) {
	case RendererType::VULKAN: {
		VulkanRenderer* vk = static_cast<VulkanRenderer*>(cntx->renderer);
		auto device = vk->getDevice();
		if (systemDataUBOOutOfDate) {
			vk->UpdateUniformBuffer<SYS_LIGHT_DATA>(data, systemDataUBO);
			systemDataUBOOutOfDate = false;
		}
		vkWaitForFences(device, 1, &Fence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &Fence);
		vk->CMDBeginRecord(cmd);
		vk->CMDRecordBindPipeline(cmd, CL_Pipelineinfo.pipeline, VK_PIPELINE_BIND_POINT_COMPUTE);
		vk->CMDRecordDescriptorSet(cmd, CL_Pipelineinfo.pipelineLayout, VK_PIPELINE_BIND_POINT_COMPUTE, &CL_DescriptorSetInfo.descriptorSet[frameIndex]);
		vk->CMDRecordDistpatch(cmd, LIGHT_SYS_CONST::GROUP_COUNT, 1, 1);
		vk->CMDEndRecord(cmd);
		
		vk->CMDSubmitComputeQueue(&cmd, 1, Fence, nullptr, nullptr,0,&SignalSema,1);
		break;
	}
	}
}

void SYS_Light::ScreenResizeCameraEvent(int width, int height)
{
	
	// next four  var are dependent on the Camera
	// if the cameras projection matrix changes (most likly due to screen size change)
	// these values should get updated.
	if (!isInit)
		return;
	auto comp = camera.lock();
	if (!comp)
		return;
	auto cam = std::dynamic_pointer_cast<CActor>(comp)->GetComponent<CCamera>();

	data.inverseProjection = MMath::inverse(cam->GetProjection());
	auto planes = cam->GetzPlanes();
	data.zPlanes[0] = planes.x;
	data.zPlanes[1] = planes.y;
	data.screenDimensions[0] = static_cast<uint32_t>(width);
	data.screenDimensions[1] = static_cast<uint32_t>(height);
	systemDataUBOOutOfDate = true;
}

bool SYS_Light::RegisterLight(CLight* Light)
{ 
	if (!mapppedLightSSBO || !isInit)
		return false;
	// ADD LIGHT TO SSBO
	if(LightCount >= LightCapacity)
		return false;
	if (HandelsMap.find(Light->ssboIndex) != HandelsMap.end())
		return false;
	//Update interal map
	HandelsMap[LightCount] = Light;
	Light->ssboIndex = LightCount;
	
	CLightData ldata = Light->GetUpdatedData();	
	// index to the place in the array and copy over the data
	size_t offset = LightCount * sizeof(CLightData);
	memcpy((char*)mapppedLightSSBO + offset, &ldata, sizeof(CLightData));
	

	LightCount++;
	data.lightCount = LightCount;
	systemDataUBOOutOfDate = true;
	return true;
}

bool SYS_Light::DeregisterLight(CLight* Light)
{
	// REMOVE LIGHT FORM SSBO
	if (!mapppedLightSSBO || !isInit)
		return false;

	if (LightCount == 0)
		return false;
	if (HandelsMap.find(Light->ssboIndex) == HandelsMap.end())
		return false;

	if (LightCount > 1) { // for more thant 1 element 
		uint32_t removeHandel = Light->ssboIndex;
		uint32_t lastLightIndex = LightCount - 1;

		if (removeHandel != lastLightIndex) { // check if the element is not the last one
			CLight* last = HandelsMap[lastLightIndex];
			HandelsMap.erase(lastLightIndex);
			HandelsMap[removeHandel] = last;
			last->ssboIndex = removeHandel;
			CLightData ldata = last->GetUpdatedData();
			size_t offset = removeHandel * sizeof(CLightData);
			memcpy((char*)mapppedLightSSBO + offset, &ldata, sizeof(CLightData));
		}
		else {
			HandelsMap.erase(removeHandel);		
		}
	}
	else { // for the case of 1 element left
		HandelsMap.clear();	
	}
	LightCount--;
	data.lightCount = LightCount;
	systemDataUBOOutOfDate = true;
	return true;
}

bool SYS_Light::UpdateLightData(CLight* Light)
{
	if (!mapppedLightSSBO || !isInit)
		return false;
	// UPDATE LIGHT DATA AT CURRENT INDEX IN THE SSBO
	if(HandelsMap.find(Light->ssboIndex) == HandelsMap.end())
		return false;
	CLightData ldata = Light->GetUpdatedData();
	size_t offset = Light->ssboIndex * sizeof(CLightData);
	memcpy((char*)mapppedLightSSBO + offset, &ldata, sizeof(CLightData));

	return true;
}
