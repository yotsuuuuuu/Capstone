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

	// TODO:
	// get piepe
	// get set
	// get terrain chunk

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float dt) {}
};