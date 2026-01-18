#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include "CoreStructs.h"
#include "Debug.h"


class VkImGUISystem
{
	//Copying and moving disabled
	VkImGUISystem(const VkImGUISystem&) = delete;
	VkImGUISystem(VkImGUISystem&&) = delete;
	VkImGUISystem& operator=(const VkImGUISystem&) = delete;
	VkImGUISystem& operator=(VkImGUISystem&&) = delete;

	ImGuiContex context;
	VkDescriptorPool imguiDescriptorPool;
	
	
public:

	VkImGUISystem();
	~VkImGUISystem();	

	bool Initialize(const ImGuiContex& cntx);
	void RecordCMDBuffer(const VkCommandBuffer& cmd);
	void ImGUIHandelEvents(const SDL_Event& event);
	void BeginFrame();
	void EndFrame();
	
	void TestUI();

	static void CheckVkResult(VkResult err) {
		if (err != VK_SUCCESS) {
			Debug::Error("VkImGUISystem: VkResult is " + std::to_string(err), __FILE__, __LINE__);
		}
	}
private:
	
	void CreateDescriptorPool();
};

