#include "VkImGUISystem.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "SYS_Light.h"
#include "FmodController.h"
#include "VulkanRenderer.h"



VkImGUISystem::VkImGUISystem():imguiDescriptorPool(VK_NULL_HANDLE)
{
    CustomEvent::Initilize();
}

VkImGUISystem::~VkImGUISystem()
{

    
}

bool VkImGUISystem::Initialize(const ImGuiContex& cntx)
{
   
    context = cntx;
    if (imguiDescriptorPool == VK_NULL_HANDLE) {
        CreateDescriptorPool();
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();  
	style.ScaleAllSizes(cntx.main_scale);
	style.FontScaleDpi = cntx.main_scale;

    ImGui_ImplSDL3_InitForVulkan(cntx.window);
    ImGui_ImplVulkan_InitInfo info{};
	info.ApiVersion = cntx.version;
	info.Instance = cntx.instance;
	info.PhysicalDevice = cntx.physicalDevice;
	info.Device = cntx.device;
	info.CheckVkResultFn = VkImGUISystem::CheckVkResult;
	info.QueueFamily = cntx.queueFamily;
	info.Queue = cntx.queue;
	info.DescriptorPool = imguiDescriptorPool;
	info.ImageCount = cntx.numberofSwapchains;
	info.MinImageCount = cntx.framesInFlight;
	info.PipelineInfoMain.RenderPass = cntx.mainPass;
	info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	info.PipelineInfoMain.Subpass = 0;
    

    return  ImGui_ImplVulkan_Init(&info);
}

void VkImGUISystem::ShutDonw() {
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(context.device, imguiDescriptorPool, nullptr);
    imguiDescriptorPool = VK_NULL_HANDLE;

    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void VkImGUISystem::RecordCMDBuffer(const VkCommandBuffer& cmd)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void VkImGUISystem::ImGUIHandelEvents(const SDL_Event& event, const EngineContext& cntx)
{
    switch (event.type) {
        case SDL_EVENT_KEY_UP:
            switch (event.key.scancode) {
            case SDL_SCANCODE_P: {
                ShowSongMenu = !ShowSongMenu;
                SDL_Event customEvent;
                SDL_zero(customEvent);
                customEvent.type = CustomEvent::AUDIO_MENU_EVENT;
                customEvent.user.code = ShowSongMenu ? 1 : 0; // 1 = menu open, 0 = menu closed
                SDL_PushEvent(&customEvent);
            }
                break;
            }

        break;
    }
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void VkImGUISystem::BeginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void VkImGUISystem::EndFrame()
{
    ImGui::Render();
}

void VkImGUISystem::GatherSystemData(const EngineContext& cntx)
{
    int numberOfSongs = 0;
    while (true) {
        std::string  name = cntx.fmodController->getSongName(numberOfSongs);
        
        if (name.empty()) break; // stops when all the songs have been found
        
        SongNameList[numberOfSongs] = name;
        numberOfSongs++;
    }
}

void VkImGUISystem::SystemUI(const EngineContext& cntx)
{
    // Display Audio Visualler/ Current Name Of the Song // Current time // Controllo audio // Stop and Start

    const AudioBands& bands = cntx.fmodController->GetFrameAudioBand();
    SongTime time = cntx.fmodController->getCurrentTime();
    std::string currentname;
    if (currentSongIndex != -1) {  
        currentname = SongNameList[currentSongIndex];
        ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Always); 
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Audio", nullptr, ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoResize|
            ImGuiWindowFlags_NoBackground);
        ImGui::Text("CurrentLy Playing: "); ImGui::SameLine();
        ImGui::Text(currentname.c_str());
        ImGui::Text("Song Time: %02d:%02d/%02d:%02d", time.min, time.sec, SongLenght.min, SongLenght.sec);
        //ImGui::PlotLines("##Audio", (float*)&bands, 8, 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        float width = 200, height = 80.0f;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImColor BottomColor = IM_COL32(23, 134, 134, 255);
        ImColor TopColor = IM_COL32(255, 0, 0, 255);
        float* bandsData = (float*)&bands;
        int numberofBands = 9;
        for (int i = 0; i < numberofBands - 1 ; i++) {
            float x0 = canvasPos.x + (i / 7.0f) * width;
            float x1 = canvasPos.x + ((i + 1) / 7.0f) * width;
            float y0 = canvasPos.y + height - (bandsData[i] * height);
            float y1 = canvasPos.y + height - (bandsData[i + 1] * height);
            float cx = (x0 + x1) * 0.5f;
            
            float t = (bandsData[i] + bandsData[i + 1]) * 0.5f;
            ImColor segmentColor = ImLerp((ImVec4)BottomColor, (ImVec4)TopColor, t);

            draw_list->AddBezierCubic(
                ImVec2(x0, y0), ImVec2(cx, y0),
                ImVec2(cx, y1), ImVec2(x1, y1),
                segmentColor, 2.0f
            );
        }
        ImGui::Dummy(ImVec2(width, height));
        ImGui::Text("Press P for Pause Menu");
        ImGui::End();
    }
    SDL_SetWindowRelativeMouseMode(dynamic_cast<VulkanRenderer*>(cntx.renderer)->getWindow(), !ShowSongMenu);
    if (ShowSongMenu) {
        ImVec2 window_size = ImVec2(400, 250);
        ImVec2 display_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowSize(window_size);
        ImGui::SetNextWindowPos(ImVec2((display_size.x - window_size.x) * 0.45f,
            (display_size.y - window_size.y) * 0.45f));
        ImGui::Begin("Pause Menu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize );
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
        ImGuiStyle& style = ImGui::GetStyle();
        float child_w = (ImGui::GetContentRegionAvail().x - 1 * style.ItemSpacing.x) / 1.75f;        
        ImGui::BeginChild("##SonglistNames", ImVec2(child_w, 200.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::BeginMenuBar();
        ImGui::TextUnformatted("SongList");
        ImGui::EndMenuBar();
       
        for (auto it = SongNameList.begin(); it != SongNameList.end(); ++it) {
            ImGui::PushID(it->first);
            if (ImGui::Selectable("##selecteable")) {
                currentSongIndex = it->first;                
                cntx.fmodController->playsong(currentSongIndex);
                SongLenght = cntx.fmodController->getTimeOfSong(currentSongIndex);
                // need to change the world here                               
            }
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(it->second.c_str());              
            ImGui::PopID();

        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::SetNextItemWidth(child_w);
        static float volume = 25.0f; // 0 - 100
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 12.0f);
        if (ImGui::VSliderFloat("##Volume",ImVec2(20,200), &volume, 0.0f, 100.0f, "")) {
            cntx.fmodController->Volume(volume);
        }
        ImGui::PopStyleVar();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Volume: %.0f", volume);
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        if (ImGui::Button("Play", ImVec2(child_w * 0.5f, 0))) {             
            cntx.fmodController->playsong(AudioState::PLAY);
        }
        if (ImGui::Button("Pause", ImVec2(child_w * 0.5f, 0))) {
            
            cntx.fmodController->playsong(AudioState::PAUSE);
        }
        ImGui::EndGroup();
        ImGui::PopStyleVar();
       
        ImGui::End();
    }
  

}

void VkImGUISystem::TestUI()
{
    //ImGui::ShowDemoWindow();
    ImGuiIO& io = ImGui::GetIO();   
    ImGui::Begin("Fps", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%.3f ms/frame (%.1f FPS) ", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();   

}

void VkImGUISystem::CreateDescriptorPool()
{

    // making a pool for descriptors that is up too 80 
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE * 10 },
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : pool_sizes)
        pool_info.maxSets += pool_size.descriptorCount;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(context.device, &pool_info, nullptr, &imguiDescriptorPool);
}