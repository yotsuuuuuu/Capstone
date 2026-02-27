#pragma once
#include "CoreStructs.h"
#include "CLight.h"
#include "EngineContext.h"

class SYS_Light
{
	//TODO (Kev) Light system
	// creates Light SSBO that holds the light data gobally loaded
	// probly going to need an UBO for light system info
	//  Able to add/remove/and update the SSBO
	//	It should give back the handle back to the light component to hold
	//  and light system should hold a weakpointer to it so it can call funtctions form it and set values but dosent own their lifetime
	//  Hold the Compute Logic for clusters  and cluster SSBO
	// there needs Caculate the Clusters every frame
	//  to do that it needs needs the cameras proj and view matrix,
	//  also needs the SSBO of Light and Clusters
	BufferMemory ScreenClustersSSBO;
	uint32_t ClusterCount;
	SYS_LIGHT_DATA data;
	BufferMemory systemDataUBO;
	BufferMemory ActiveSceneLightSSBO;
	uint32_t LightCount;
	uint32_t LightCapacity;
	//TODO: add pipelines and descriptor sets
	// need 1 for Computing the Clusters
	PipelineInfo CC_Pipelineinfo;
	DescriptorSetInfo CC_DescriptorSetInfo;
	// need 1 for Computing the Lights in the Clusters
	PipelineInfo CL_Pipelineinfo;
	DescriptorSetInfo CL_DescriptorSetInfo;

	EngineContext* cntx;

	std::unordered_map<uint32_t, CLight* > HandelsMap;

public:
	SYS_Light(EngineContext* cntx_, uint32_t ClusterCapacity, uint32_t LightCapacity_)
		: cntx(cntx_), ClusterCount(ClusterCapacity), LightCapacity(LightCapacity_), LightCount(0), ScreenClustersSSBO({}),
		systemDataUBO({}), ActiveSceneLightSSBO({}), CC_Pipelineinfo({}), CL_Pipelineinfo({}) {}

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
	void ComputeLightClusters();

	bool RegisterLight(CLight* Light);
	bool DeregisterLight(CLight* Light);
	bool UpdateLightData(CLight* Light);

};

