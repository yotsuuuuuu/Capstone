#pragma once
#include "CoreStructs.h"
#include "CLight.h"
#include "EngineContext.h"

class SYS_Light
{
	WeakRef<Component> mesh;
	WeakRef<Component> mat; 
	//TODO (Kev) Light system
	// NEED TO ACCOUNT FOR RESIZEING OF THE SCREEN
	BufferMemory ScreenClustersSSBO;
	uint32_t ClusterCount;
	SYS_LIGHT_DATA data;
	bool systemDataUBOOutOfDate = false;
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
	std::vector<VkCommandBuffer> ComputeCmd;
	VkCommandPool ComputePool;
	//sync objects
	VkSemaphore SignalSema;	
	VkFence Fence;

	bool isInit = false;
	

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

	void ScreenResizeCameraEvent(int width, int height);

	bool RegisterLight(CLight* Light);
	bool DeregisterLight(CLight* Light);
	bool UpdateLightData(CLight* Light);

	VkSemaphore GetLightCullReadySingal() { return SignalSema; }

	uint32_t GetCurrentLightCount() { return data.lightCount; }

	std::vector< BufferMemory> GetSysUBO() { return { systemDataUBO }; }
	std::vector< BufferMemory> GetClusterSSBO() { return { ScreenClustersSSBO }; }
	std::vector< BufferMemory> GetLightSSBO() { return { ActiveSceneLightSSBO }; }

	void SetMesh(Ref<Component> mesh_) { mesh = mesh_;}
	void SetMat(Ref<Component> mat_) { mat = mat_;}

	Ref<Component> GetMesh() { return mesh.lock(); }
	Ref<Component> GetMat() { return mat.lock(); }
	
};

