#pragma once
#include "Component.h"
#include <string>
#include <vector>
#include "CoreStructs.h"

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

	
	SkyboxPush push;

public:
	CSkyBox(Ref<Component> parent_, Renderer* renderer_, std::vector<std::string> files ): Component(parent_),
		renderer(renderer_),paths(files), CubeSampler({})  
	{
		push.Bloomfactor = 1.0f;
		push.ColorTint = Vec4(0.45, 0.475, 0.5, 1.0);
	}


	bool OnCreate() override;
	void OnDestroy()  override;
	void Update(const float dt)  override {}

	IndexedVertexBuffer GetMesh();

	void RecreatePipeline();
	PipelineInfo GetPipeline();
	std::vector<VkDescriptorSet> GetSet() { return DesSet; }

	void ImGui();

	SkyboxPush GetSkyBoxPush() { return push; }
	void SetSkyBoxPush(SkyboxPush p) { push = p; }

};

