#pragma once
#include "Component.h"
#include "CoreStructs.h"
#include <string>

class CShader; 
class Renderer;

class CMaterial : public Component
{
	WeakRef<CShader> shader;
	std::vector<std::string> textureFilePaths;
	std::vector <Sampler2D> textures;	
	std::vector<VkDescriptorSet> descriptionSet;
	Renderer* render;
	uint32_t setId;

public:
	CMaterial(Ref<Component> parent_, Renderer* render_, std::vector<std::string> FilePaths, Ref<CShader> cShader)
		: Component(parent_), render(render_), textureFilePaths(FilePaths), shader(cShader), setId(1) {
	}
	virtual ~CMaterial() {}

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	

	PipelineInfo GetPipelineInfo();
	
	//void SetTextureSampler(const Sampler2D& text) {	texture = text;	}
	//Sampler2D GetTextureSampler() { return texture; }
	std::vector<VkDescriptorSet> GetDescriptorSet() { return descriptionSet; }


	void SetRenderSetValue(uint32_t id) { setId = id; }
	uint32_t GetRednerSetValue() { return setId; }
};

