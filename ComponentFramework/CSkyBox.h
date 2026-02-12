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

public:
	CSkyBox(Ref<Component> parent_, Renderer* renderer_, std::vector<std::string> files ): Component(parent_),
		renderer(renderer_), CubeSampler({}), DesSet(VK_NULL_HANDLE) { }


	bool OnCreate() override;
	void OnDestroy()  override;
	void Update(const float dt)  override {}


};

