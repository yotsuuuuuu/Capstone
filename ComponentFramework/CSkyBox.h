#pragma once
#include "Component.h"
#include <string>
#include <vector>
#include "CoreStructs.h"
#include "EngineContext.h"

class Renderer;
class CSkyBox : public Component
{
	//mesh
	Ref<Component> Mesh;
	// shader
	Ref<Component> Shader;
	// the materials
	std::vector<std::string> paths;
	Sampler2D CubeSampler;
	std::vector<VkDescriptorSet> DesSet;
	Renderer* renderer;

	float m_smoothed1 = 0.0f;
	float m_smoothed2 = 0.0f;
	
	SkyboxPush push;

public:
	CSkyBox(Ref<Component> parent_, Renderer* renderer_, std::vector<std::string> files ): Component(parent_),
		renderer(renderer_),paths(files), CubeSampler({})  
	{
		push.Bloomfactor = 1.0f;
		push.ColorTint = Vec4(0.610f, 0.707f, 0.803f, 1.0f);
	}


	bool OnCreate() override;
	void OnDestroy()  override;
	void Update(const float dt)  override {}

	IndexedVertexBuffer GetMesh();

	void RecreatePipeline();
	PipelineInfo GetPipeline();
	std::vector<VkDescriptorSet> GetSet() { return DesSet; }


	void AudioReact(EngineContext& cntx);

	void ImGui();

	SkyboxPush GetSkyBoxPush() { return push; }
	void SetSkyBoxPush(SkyboxPush p) { push = p; }

};

