#pragma once
#include "World.h"
#include "Component.h"
//TODO: (andres) MAKE A WRAPPER FOR THIS CLASS SO THAT WE CAN USE IT AS A COMPONENT IN THE ECS
class Renderer;

class CWorld : public Component {

	World* C_World;
	TerrainPreset worldConfig;
	Ref<Component> Shader;
	std::vector<VkDescriptorSet> Descriptorset;


public:
	CWorld(Ref<Component> parent_, Renderer* renderer_, const TerrainPreset& config) : Component(parent_),
	worldConfig(config){
		C_World = new World(renderer_);
	}
	~CWorld();

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float dt) {}

	// TODO:
	// get piepe
	// get set
	// get terrain chunk

	//PipelineInfo const GetWorldPipeline() { return C_World->GetPipeline(); }
	//std::vector<VkDescriptorSet> const GetWorldDescriptorSet() { return C_World->GetDescriptorSetInfo().descriptorSet; }
	std::unordered_map<Vec2, TerrainChunkData> GetChunkRenderData() { return C_World->GetChunkRenderData(); }

	void InitializeWorld(TerrainPreset* t_);

};