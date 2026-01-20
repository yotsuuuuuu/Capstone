#pragma once
#include "Component.h"
#include "CoreStructs.h"

class Renderer;

class CMaterial : public Component
{
	const char* textureFile;
	Sampler2D texture;
	Renderer* renderer;
	//not sure
	// if the Material should own the descriptor set info
	// or the renderer should manage it
	// or have a dedicated material manager
	// or have a rendering component that has the descriptor set info
	DescriptorSetInfo descriptorSetInfo;

public:
	CMaterial(Ref<Component> parent_,Renderer* renderer_, const char* textureFile_ = nullptr)
		: Component(parent_),renderer(renderer_), textureFile(textureFile_), texture({}), descriptorSetInfo({}) {
	}
	virtual ~CMaterial() {}

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}



};

