#pragma once
#include "Component.h"
#include "CoreStructs.h"
#include <string>

class Renderer;

class CShader : public Component
{
	std::string vertShaderFile;
	std::string fragShaderFile;
	std::string geomShaderFile;
	std::string tesCShaderFile;
	std::string tesEShaderFile;
	Renderer* render;
	uint32_t poolSize;
	PipelineInfo pipelineInfo;
	DescriptorSetInfo desInfo;

	std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
public:
	CShader(Ref<Component> parent_, Renderer* render_, std::vector<SingleDescriptorSetLayoutInfo> layoutInfo_, std::string vertShaderFile_, std::string fragShaderFile_
		, std::string geomShaderFile_ = "", std::string tesCShaderFile_ = "", std::string tesEShaderFile_ = "")
		: Component(parent_),render(render_), layoutInfo(layoutInfo_), pipelineInfo({}), vertShaderFile(vertShaderFile_),
		fragShaderFile(fragShaderFile_),geomShaderFile(geomShaderFile_),
		tesCShaderFile(tesCShaderFile_),tesEShaderFile(tesEShaderFile_), poolSize(100){
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

