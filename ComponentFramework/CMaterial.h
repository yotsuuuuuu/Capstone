#pragma once
#include "Component.h"
#include "CoreStructs.h"


class CShader; 
class Renderer;

class CMaterial : public Component
{
	const char* textureFile;
	Sampler2D texture;
	WeakRef<CShader> shader;
	//not sure
	// if the Material should own the descriptor set info
	// or the renderer should manage it
	// or have a dedicated material manager
	// or have a rendering component that has the descriptor set info
	std::vector<VkDescriptorSet> descriptionSet;
	Renderer* render;
	uint32_t setId;

public:
	CMaterial(Ref<Component> parent_,Renderer* render_, const char* textureFile_ = nullptr)
		: Component(parent_), render(render_), textureFile(textureFile_), texture({}),setId(1) {
	}
	virtual ~CMaterial() {}

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	void SetDescriptorSet(const std::vector<VkDescriptorSet>& set) { descriptionSet = set; }
	std::vector<VkDescriptorSet> GetDescriptorSet() { return descriptionSet; }
	void SetShader(Ref<CShader> shade) { shader = shade; }
	PipelineInfo GetPipelineInfo();
	
	void SetTextureSampler(const Sampler2D& text) {	texture = text;	}
	Sampler2D GetTextureSampler() { return texture; }



	void SetRenderSetValue(uint32_t id) { setId = id; }
	uint32_t GetRednerSetValue() { return setId; }
};

