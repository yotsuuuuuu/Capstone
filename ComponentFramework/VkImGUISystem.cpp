#include "VkImGUISystem.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"



VkImGUISystem::VkImGUISystem():imguiDescriptorPool(VK_NULL_HANDLE)
{
}

VkImGUISystem::~VkImGUISystem()
{

    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(context.device, imguiDescriptorPool, nullptr);

    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
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

void VkImGUISystem::RecordCMDBuffer(const VkCommandBuffer& cmd)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void VkImGUISystem::ImGUIHandelEvents(const SDL_Event& event)
{
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

void VkImGUISystem::TestUI()
{
    ImGui::ShowDemoWindow();
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