#pragma once
#include "Component.h"
#include "CoreStructs.h"

class Renderer;

class CShader : public Component
{
	const char* vertShaderFile;
	const char* fragShaderFile;
	const char* geomShaderFile;
	const char* tesCShaderFile;
	const char* tesEShaderFile;
	Renderer* render;
	uint32_t poolSize;
	PipelineInfo pipelineInfo;
	DescriptorSetInfo desInfo;

	std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
public:
	CShader(Ref<Component> parent_, Renderer* render_, std::vector<SingleDescriptorSetLayoutInfo> layoutInfo_,const char* vertShaderFile_, const char* fragShaderFile_
		,const char* geomShaderFile_ = nullptr,const char* tesCShaderFile_ = nullptr,const char* tesEShaderFile_ = nullptr)
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

