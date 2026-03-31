#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include "CoreStructs.h"
#include "Debug.h"
#include "EngineContext.h"
#include "CustomSDLEvents.h"
class VkImGUISystem
{
	//Copying and moving disabled
	VkImGUISystem(const VkImGUISystem&) = delete;
	VkImGUISystem(VkImGUISystem&&) = delete;
	VkImGUISystem& operator=(const VkImGUISystem&) = delete;
	VkImGUISystem& operator=(VkImGUISystem&&) = delete;

	ImGuiContex context;
	VkDescriptorPool imguiDescriptorPool;


	int currentSongIndex = -1;
	std::unordered_map<int,std::string> SongNameList;
	SongTime SongLenght;
	
	bool ShowSongMenu = true;
	
public:

	VkImGUISystem();
	~VkImGUISystem();	

	bool Initialize(const ImGuiContex& cntx);
	void ShutDonw();

	void RecordCMDBuffer(const VkCommandBuffer& cmd);
	void ImGUIHandelEvents(const SDL_Event& event,const EngineContext& cntx);
	void BeginFrame();
	void EndFrame();
	
	void GatherSystemData(const EngineContext& cntx);

	void SystemUI(const EngineContext& cntx);

	void TestUI();

	static void CheckVkResult(VkResult err) {
		if (err != VK_SUCCESS) {
			Debug::Error("VkImGUISystem: VkResult is " + std::to_string(err), __FILE__, __LINE__);
		}
	}
private:
	
	void CreateDescriptorPool();
};

