#pragma once
#include "CoreStructs.h"
#include "CLight.h"
#include "EngineContext.h"

class SYS_Light
{
	//TODO (Kev) Light system
	// CREATE THE COMPUTE SHADERS 
	// CREATE SYSTEM IN THE SCENE MANAGER
	// IMPLEMENT INTO THE MAIN REDNER LOOP
	// UPDATE MAIN SHADERS TO USE THE LIGHT SYSTEM.
	// NEED TO ACCOUNT FOR RESIZEING OF THE SCREEN
	BufferMemory ScreenClustersSSBO;
	uint32_t ClusterCount;
	SYS_LIGHT_DATA data;
	bool systemDataUBOOutOfDate;
	BufferMemory systemDataUBO;
	BufferMemory ActiveSceneLightSSBO;
	void* mapppedLightSSBO;
	uint32_t LightCount;
	uint32_t LightCapacity;
	// need 1 for Computing the Clusters
	PipelineInfo CC_Pipelineinfo;
	DescriptorSetInfo CC_DescriptorSetInfo;
	// need 1 for Computing the Lights in the Clusters
	PipelineInfo CL_Pipelineinfo;
	DescriptorSetInfo CL_DescriptorSetInfo;
	// also need semaphors and maybe a command buffer so it can be submitted own its own an
	// sync with the semaphore
	VkCommandBuffer ComputeCmd;
	VkCommandPool ComputePool;
	//sync objects
	VkSemaphore SignalSema;	
	VkFence Fence;
	

	EngineContext* cntx;
	WeakRef<Component> camera;

	std::unordered_map<uint32_t, CLight* > HandelsMap;

public:
	SYS_Light(EngineContext* cntx_, uint32_t LightCapacity_);
		

	~SYS_Light();

	// Initilize all the SSBOs, UBOS,
	// 
	bool Initilize();
	void ShutDonw();
	
	// Calculates the Clusters AABB box based on the projection matrix
	// if cameras projection Matrix ever changes 
	// This has to be reCalculated
	void ComputeClusters();

	// Calcualtes Which ligths Affects which Clusters
	// Based on the Cameras view Matrix
	// has to be done everyframe.
	void ComputeLightClusters(uint32_t frameIndex);

	bool RegisterLight(CLight* Light);
	bool DeregisterLight(CLight* Light);
	bool UpdateLightData(CLight* Light);

	// TODO ADD FUNCTIONS TO : CHANGE OF SCREEN SIZE

};

