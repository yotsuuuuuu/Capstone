#pragma once
#include "Component.h"
#include "CoreStructs.h"
#include <string>

class Renderer;

class CShader : public Component
{
	std::string vertShaderFile;
	std::string fragShaderFile;
	std::optional<std::string> geomShaderFile;
	std::optional<std::string> tesCShaderFile;
	std::optional<std::string> tesEShaderFile;
	Renderer* render;
	uint32_t poolSize;
	std::optional<PipeLineConfig> p_config;
	PipelineInfo pipelineInfo;
	DescriptorSetInfo desInfo;

	std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
public:
	CShader(Ref<Component> parent_, Renderer* render_, std::vector<SingleDescriptorSetLayoutInfo> layoutInfo_, 
		std::string vertShaderFile_, std::string fragShaderFile_, std::optional<PipeLineConfig> config_ = std::nullopt
		, std::optional<std::string> geomShaderFile_ = std::nullopt, std::optional<std::string> tesCShaderFile_ = std::nullopt,
		std::optional<std::string> tesEShaderFile_ = std::nullopt)
		: Component(parent_),render(render_), layoutInfo(layoutInfo_), pipelineInfo({}), p_config(config_), vertShaderFile(vertShaderFile_),
		fragShaderFile(fragShaderFile_),geomShaderFile(geomShaderFile_),
		tesCShaderFile(tesCShaderFile_),tesEShaderFile(tesEShaderFile_), poolSize(30){
	}

	
	virtual ~CShader() {}

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}	
	
	PipelineInfo GetPipelineInfo() const { return pipelineInfo; }	
	void SetPipelineInfo(const PipelineInfo& pipelineInfo_) { pipelineInfo = pipelineInfo_; }

	std::vector<VkDescriptorSet> AllocateDescriptorSet(std::vector<Sampler2D> arrySampler);

	void RecreatePipeLine();

};

