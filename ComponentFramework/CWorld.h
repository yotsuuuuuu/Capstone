#pragma once
#include "World.h"
#include "Component.h"
class Renderer;

class CWorld : public Component {

	World* C_World;
	TerrainPreset worldConfig;
	Ref<Component> Shader;
	std::vector<VkDescriptorSet> Descriptorset;


public:
	CWorld(Ref<Component> parent_, EngineContext& engineContext_, const TerrainPreset& config) : Component(parent_),
		worldConfig(config) {
		C_World = new World(engineContext_);
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
	std::unordered_map<Vec3, TerrainChunkData>* GetChunkRenderData() { return C_World->GetChunkRenderData(); }
	std::vector<std::unique_ptr<Chunk>>* GetChunks() { return C_World->GetChunks(); }
	void InitializeWorld(TerrainPreset* t_);
	void InitializeWorld(int songIndex);

};