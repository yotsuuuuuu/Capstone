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

	PipelineInfo pipelineInfo;
	std::vector<VkDescriptorSetLayout> pipeLineLayout;
public:
	CShader(Ref<Component> parent_, Renderer* render_, std::vector<VkDescriptorSetLayout> pipelay ,const char* vertShaderFile_, const char* fragShaderFile_
		,const char* geomShaderFile_ = nullptr,const char* tesCShaderFile_ = nullptr,const char* tesEShaderFile_ = nullptr)
		: Component(parent_),render(render_),pipeLineLayout(pipelay), pipelineInfo({}), vertShaderFile(vertShaderFile_),
		fragShaderFile(fragShaderFile_),geomShaderFile(geomShaderFile_),
		tesCShaderFile(tesCShaderFile_),tesEShaderFile(tesEShaderFile_) {
	}
	virtual ~CShader() {}

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}	
	
	PipelineInfo GetPipelineInfo() const { return pipelineInfo; }	
	void SetPipelineInfo(const PipelineInfo& pipelineInfo_) { pipelineInfo = pipelineInfo_; }

	void RecreatePipeLine();

};

