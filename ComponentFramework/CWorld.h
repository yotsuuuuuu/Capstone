#pragma once
#include "Component.h"
#include "World.h"

class CWorld : public Component
{
public:
	CWorld(Ref<Component> parent_ = nullptr, Renderer* renderer_ = nullptr) : Component(parent_), renderer(renderer_), world(nullptr){}
	~CWorld() {}
	bool OnCreate() override;
	void OnDestroy() override;
	void Update(const float dt) override;
private:
	World* world;
	Renderer* renderer;

	void OnIntializeWorld(TerrainPreset* t_, std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_);

	//class VulkanRenderer* vRenderer;

};

