#include "VkImGUISystem.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "SYS_Light.h"
#include "FmodController.h"
#include "VulkanRenderer.h"
#include "AssetManager.h"
#include "CActor.h"
#include "CSkyBox.h"



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


static bool ShowSkyBoxColorEditor = false;
static bool ShowTerrainColorEditor = false;

void VkImGUISystem::ImGUIHandelEvents(const SDL_Event& event, const EngineContext& cntx)
{
    if (event.type == CustomEvent::SONG_SELECTED_EVENT) {
        cntx.fmodController->playsong(currentSongIndex);
        SongLenght = cntx.fmodController->getTimeOfSong(currentSongIndex);
    }
    switch (event.type) {
        case SDL_EVENT_KEY_UP:
            switch (event.key.scancode) {
            case SDL_SCANCODE_TAB: {
                ShowSongMenu = !ShowSongMenu;
                ShowSkyBoxColorEditor = false ;
                ShowTerrainColorEditor = false;
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
    ImColor BottomColor = IM_COL32(23, 134, 134, 255);
    ImColor TopColor = IM_COL32(255, 0, 0, 255);
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
        ImGui::Text("Press TAB for Pause Menu");
        ImGui::End();
    }
    SDL_SetWindowRelativeMouseMode(dynamic_cast<VulkanRenderer*>(cntx.renderer)->getWindow(), !ShowSongMenu);
    if (ShowSongMenu) {
        ImVec2 window_size = ImVec2(400, 280);
        ImVec2 display_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowSize(window_size);
        ImGui::SetNextWindowPos(ImVec2((display_size.x - window_size.x) * 0.45f,
            (display_size.y - window_size.y) * 0.45f));
        float  alpha = 1.0f;
        if (ShowSkyBoxColorEditor || ShowTerrainColorEditor) {
            alpha = 0.5f;
            
        }
        ImGui::SetNextWindowBgAlpha(alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
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
             
                // need to change the world here      
                SDL_Event customEvent;
                SDL_zero(customEvent);
                customEvent.type = CustomEvent::SONG_SELECTED_EVENT;
                customEvent.user.code = (Sint32)currentSongIndex; 
                SDL_PushEvent(&customEvent);
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
        ImVec4 color =  ImLerp((ImVec4)BottomColor, (ImVec4)TopColor, volume / 100.0f);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, color);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, color);
        if (ImGui::VSliderFloat("##Volume",ImVec2(20,200), &volume, 0.0f, 100.0f, "")) {
            cntx.fmodController->Volume(volume);
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Volume: %.0f", volume);
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        if (ImGui::Button("Reload Song List", ImVec2(child_w * 0.5f, 0))) {
            cntx.fmodController->AddSonginFile();
            cntx.fmodController->InitilizeSongs();
            GatherSystemData(cntx);
        }
       
        ImGui::Dummy(ImVec2(child_w * 0.5f, 5));

        if (ImGui::Button("Play", ImVec2(child_w * 0.5f, 0))) {             
            cntx.fmodController->playsong(AudioState::PLAY);
        }
        if (ImGui::Button("Pause", ImVec2(child_w * 0.5f, 0))) {
            
            cntx.fmodController->playsong(AudioState::PAUSE);
        }
        ImGui::Dummy(ImVec2(child_w * 0.5f, 5));
        if (ImGui::Button("Reset Player", ImVec2(child_w * 0.5f, 0))) {
            SDL_Event customEvent;
            SDL_zero(customEvent);
            customEvent.type = CustomEvent::PLAYER_RESET_EVENT;
            customEvent.user.code = 1;
            SDL_PushEvent(&customEvent);           
        }      


        if (ImGui::Button("Close Menu", ImVec2(child_w * 0.5f, 0))) {
            ShowSongMenu = false;
            ShowSkyBoxColorEditor = false;
            ShowTerrainColorEditor = false;
            SDL_Event customEvent;
            SDL_zero(customEvent);
            customEvent.type = CustomEvent::AUDIO_MENU_EVENT;
            customEvent.user.code = ShowSongMenu ? 1 : 0; 
            SDL_PushEvent(&customEvent);
        }
        ImGui::EndGroup();
        ImGui::PopStyleVar();
        if (ImGui::TreeNode("Edit Options")) {
            if (ImGui::BeginTable("##split", 3)) {
                ImGui::TableNextColumn(); ImGui::Checkbox("SkyBox Edit", &ShowSkyBoxColorEditor);
                ImGui::TableNextColumn(); ImGui::Checkbox("Terrain Edit", &ShowTerrainColorEditor);
                ImGui::EndTable();
            }

            ImGui::TreePop();
        }
       
        ImGui::End();      
        ImGui::PopStyleVar();
        
    }

    if (ShowSkyBoxColorEditor) {
		auto skyox = std::dynamic_pointer_cast<CActor>(cntx.assetManager->GetCamera())->GetComponent<CSkyBox>();
        //skyox->ImGui();
		auto push = skyox->GetSkyBoxPush();
        ImGui::Begin("CSkyBox", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        //ImGui::SliderFloat("Bloom Factor (0 -> 2)", &push.Bloomfactor, 0.0f, 2.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        auto flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float;
        flags |= ImGuiColorEditFlags_NoSmallPreview;
        flags |= ImGuiColorEditFlags_NoAlpha;
        flags |= ImGuiColorEditFlags_NoSidePreview;
        //flags |= 

        if (ImGui::ColorPicker4("##RGB Bloom Tint", (float*)&push.ColorTint, flags)) {
			skyox->SetSkyBoxPush(push);
        }

        ImGui::End();
    }
  

    if (ShowTerrainColorEditor) {
		VulkanRenderer* vulkanRenderer = dynamic_cast<VulkanRenderer*>(cntx.renderer);
		const TerraindataUBO& Terraindata = vulkanRenderer->GetTerrainStateData();
        Vec4 maxColor = Terraindata.maxColor;
		Vec4 minColor = Terraindata.minColor;
		Vec2 MinMax = Vec2(Terraindata.min_max_lineWidth_edgeStrength.x, Terraindata.min_max_lineWidth_edgeStrength.y);
		Vec2 fadeStartEnd = Vec2(Terraindata.fadeStart_fadeEnd_gridScaleX_gridScaleY.x, Terraindata.fadeStart_fadeEnd_gridScaleX_gridScaleY.y);
        Vec2 ObjfadeStartEnd = Vec2(Terraindata.ObjectFadeStart_ObjectFadeEnd_pad_pad.x, Terraindata.ObjectFadeStart_ObjectFadeEnd_pad_pad.y);
		Vec2 gridScale = Vec2(Terraindata.fadeStart_fadeEnd_gridScaleX_gridScaleY.z, Terraindata.fadeStart_fadeEnd_gridScaleX_gridScaleY.w);
		float lineWidth = Terraindata.min_max_lineWidth_edgeStrength.z;
		float edgeStrength = Terraindata.min_max_lineWidth_edgeStrength.w;		
		

        ImGui::Begin("Terrain Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::ColorEdit3("Top Terrain Color", (float*)&maxColor, ImGuiColorEditFlags_Float)) {
			vulkanRenderer->UpdateTerrainMaxColor(maxColor);
        }
        if (ImGui::ColorEdit3("Bottom Terrain Color", (float*)&minColor, ImGuiColorEditFlags_Float)) {
			vulkanRenderer->UpdateTerrainMinColor(minColor);
        }
        if (ImGui::DragFloat2("Min/Max Height", (float*)&MinMax, 0.1f, -100.0f, 1000.0f, "%.2f")) {
            vulkanRenderer->UpdateTerrainMaxMinHieght(MinMax.x, MinMax.y);
        }
        if (ImGui::DragFloat2("Terrain Fade Start/End", (float*)&fadeStartEnd, 0.05f, 0.0f, 500.0f, "%.2f")) {
            vulkanRenderer->UpdateTerrainFade(fadeStartEnd.x, fadeStartEnd.y);
        }
        if (ImGui::DragFloat2("Object Fade Start/End", (float*)&ObjfadeStartEnd, 0.05f, 0.0f, 500.0f, "%.2f")) {
			vulkanRenderer->UpdateObjectFade(ObjfadeStartEnd.x, ObjfadeStartEnd.y);
        }
        if (ImGui::DragFloat2("Grid Size", (float*)&gridScale, 0.01f, 0.01f, 100.0f, "%.1f")) {
			vulkanRenderer->UpdateTerrainGridScale(gridScale.x, gridScale.y);
        }
        if (ImGui::DragFloat("Line Width", &lineWidth, 0.01f, 0.0f, 10.0f, "%.2f")) {
			vulkanRenderer->UpdateTerrainLineWidth(lineWidth);
        }
        if (ImGui::DragFloat("Edge Strength", &edgeStrength, 0.01f, 0.0f, 10.0f, "%.2f")) {
			vulkanRenderer->UpdateTerrainEdgeStrength(edgeStrength);
        }
        ImGui::End();


    }

}

void VkImGUISystem::TestUI()
{
    //ImGui::ShowDemoWindow();
    ImGuiIO& io = ImGui::GetIO();   
    ImVec2 window_size = ImVec2(200, 50);
    ImVec2 display_size = io.DisplaySize;
    ImGui::SetNextWindowSize(window_size);
	float widfactor = 1.0f;
	float heifactor = 0.0f;
    ImGui::SetNextWindowPos(ImVec2((display_size.x - window_size.x) * widfactor,
        (display_size.y - window_size.y) * heifactor));
    ImGui::Begin("Fps", nullptr, ImGuiWindowFlags_AlwaysAutoResize| 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBackground);
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