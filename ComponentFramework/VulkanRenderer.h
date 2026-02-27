#ifndef VULKANRENDERER_H 
#define VULKANRENDERER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_image.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <unordered_map>
#include <queue>

#include "CoreStructs.h"
#include "Renderer.h"
#include "DescriptorSetBuilder.h"
#include "VkImGUISystem.h"


#include <Vector.h>
#include <VMath.h>
#include <MMath.h>
#include <Hash.h>
#include <memory>

constexpr uint32_t SHAWDOW_SIZE = 1024;
//constexpr uint32_t SHAWDOW_SIZE = 2048;

using namespace MATH;

class Component;

#ifdef NDEBUG /// only use validation layers if in debug mode
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct QueueFamilyIndices {
    /// optional means that it contains no value until it is assigned.
    /// has_value() returns false if no value has ever been assigned. 
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
    bool isVkComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
    }
};




enum Recording {
    START,
    STOP
};

class VulkanRenderer : public Renderer {
    /// C11 precautions 
    VulkanRenderer(const VulkanRenderer&) = delete;  /// Copy constructor
    VulkanRenderer(VulkanRenderer&&) = delete;       /// Move constructor
    VulkanRenderer& operator=(const VulkanRenderer&) = delete; /// Copy operator
    VulkanRenderer& operator=(VulkanRenderer&&) = delete;      /// Move operator


public: /// Member functions
    VulkanRenderer();
    ~VulkanRenderer();

    
    SDL_Window* CreateWindow(std::string name_, int width, int height);
    bool OnCreate();
    void OnDestroy();
    void Render();
    void RecreateSwapChain();

  


    /// See VulkanSampler.cpp
    Sampler2D Create2DTextureImage(const char* textureFile);
    void createTextureImageView(Sampler2D& texture2D);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createTextureSampler(Sampler2D& texture2D);
    void DestroySampler2D(Sampler2D& texture2D);
    

    /// See DescriptorSetBuilder.cpp
    void BindDescriptorSet(VkPipelineLayout pipelineLayout, const std::vector<VkDescriptorSet> descriptorSet);
    void DestroyDescriptorSet(DescriptorSetInfo& descriptorSetInfo);
    

    /// See VulkanPipeline.cpp
    PipelineInfo CreateGraphicsPipeline(VkDescriptorSetLayout descriptorSetLayout, const char* vertFile, const char* fragFile,
        const char* tessCtrlFile = nullptr, const char* tessEvalFile = nullptr, const char* geomFile = nullptr);
    void DestroyPipeline(PipelineInfo pipeline);
    static std::vector<char> readFile(const std::string& filename);


    /// See VulkanMesh.cpp
    IndexedVertexBuffer LoadModelIndexed(const char* filename);
	// Recording Fuctions
	void RecordCommandBuffers(Recording start_stop); // located in VulkanCommandBuffer.cpp
    void BindMesh(IndexedVertexBuffer mesh);
    void BindPipeline(VkPipeline pipeline);
    void DrawIndexed(IndexedVertexBuffer mesh);
    void SetPushConstant(const PipelineInfo pipelineInfo, const Matrix4& modelMatrix_);
    void BindDescriptorSet(VkPipelineLayout pipelineLayout, const std::vector<VkDescriptorSet> descriptorSet,uint32_t setID);
    //

    void DestroyIndexedMesh(IndexedVertexBuffer mesh_);
private:
    void CreateVertexBuffer(IndexedVertexBuffer& indexedVertexBuffer, const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(IndexedVertexBuffer& indexedVertexBuffer, const std::vector<uint32_t>& indices);
   /// A helper function for createVertexBuffer()
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    

public:
    /// See VulkanCommandBuffer.cpp
    void createCommandPool();
    void CreateCommandBuffers();
    void DestroyCommandBuffers();
    
    /// Some inlined getter and setters 
    SDL_Window* getWindow() { return window; }
    VkDevice getDevice() { return device; }
    uint32_t getNumSwapchains() { return numSwapchains; }
    uint32_t getNumberOfFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
   
    // PER FRAME UBOS
    template<class T>
    std::vector<BufferMemory> CreateUniformBuffers() {
        std::vector<BufferMemory> ubo;
        VkDeviceSize bufferSize = sizeof(T);
        size_t numberOfBuffers = getNumberOfFramesInFlight();
        ubo.resize(numberOfBuffers);
        for (size_t i = 0; i < numberOfBuffers; i++) {
            ubo[i].bufferMemoryLength = bufferSize;
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                ubo[i].bufferID, ubo[i].bufferMemoryID);
        }
        return ubo;
    }
    // UPDATE ALL UBO PER FRAME
    template <class T>
    void UpdateUniformBuffers(const T& srcData, const std::vector<BufferMemory> bufferMemory) {
        void* data;
        size_t num = getNumberOfFramesInFlight();
		size_t size = sizeof(T);
		VkDeviceSize bufferSize = static_cast<VkDeviceSize>(size);

        for (size_t i = 0; i < num; ++i) {
            vkMapMemory(device, bufferMemory[i].bufferMemoryID, 0, bufferSize, 0, &data);
            memcpy(data, &srcData, static_cast<size_t>(bufferMemory[i].bufferMemoryLength));
            vkUnmapMemory(device, bufferMemory[i].bufferMemoryID);
        }
    };

    // CREATE ONE UBO
    template <class T>
    BufferMemory CreateUniformBuffer() {
        BufferMemory ubo;
        VkDeviceSize bufferSize = sizeof(T);
        ubo.bufferMemoryLength = bufferSize;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            ubo.bufferID, ubo.bufferMemoryID);
        return ubo;
    }
    // UPDATE A SPECIFIC UBO
    template <class T>
    void UpdateUniformBuffer(const T& srcData, const BufferMemory bufferMemory) {
        void* data;      
        VkDeviceSize bufferSize = static_cast<VkDeviceSize>(sizeof(T));
        vkMapMemory(device, bufferMemory.bufferMemoryID, 0, bufferSize, 0, &data);
        memcpy(data, &srcData, static_cast<size_t>(bufferMemory.bufferMemoryLength));
        vkUnmapMemory(device, bufferMemory.bufferMemoryID);
    };
  

    void DestroyUBO(std::vector<BufferMemory> ubo);


private: /// Private member variables
    const uint32_t MAX_FRAMES_IN_FLIGHT;
    uint32_t numSwapchains;
    uint32_t currentFrame;
    SDL_Event sdlEvent;
    uint32_t windowWidth;
    uint32_t windowHeight;
    SDL_Window* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkRenderPass renderPass;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

   
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
 
    CommandBufferData primaryCommandBuffer{};
    std::vector<Sampler2D> textures2D;
    std::vector<BufferMemory> cameraUboBuffers;
    std::vector<BufferMemory> lightsUboBuffers;
    std::queue<ModelMatrixPushConst> pushConstant;
    std::vector<IndexedVertexBuffer> indexedVertexBuffers;

private:
    bool hasStencilComponent(VkFormat format);
    void CreateInstance();
    void createSurface();
    void createLogicalDevice();
    void createSwapChain();
    void CreateSwapImageViews();
    
    void CreateVkLogicalDevice();
    QueueFamilyIndices VkFindQueueFamilies(VkPhysicalDevice device);
   

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createRenderPass();
    void createFramebuffers();
    void createDepthResources();
   
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    


  


    void createSyncObjects();
    void destroySwapChain();
   
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    

    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


    

    /// Used to reformat the Validation Layer's output to the console
    static void insertLineBreaks(std::string& str, size_t maxWidth) {
        size_t index = 0;
        while (index + maxWidth < str.size()) {
            /// Find a blank space before the maxWidth is encountered
            size_t breakLocation  = str.find(' ', index + maxWidth);
            if (breakLocation == std::string::npos || breakLocation <= index) {
                /// If no blank space, just break at maxWidth
                breakLocation = index + maxWidth;
            }
            str[breakLocation] = '\n';
            index = breakLocation + 1;
        }
    }


    //ImGUI system
	VkImGUISystem* imGuiSystem;
public:

    ImGuiContex GetImGuiContext();
	void ImGUIHandelEvents(const SDL_Event& event);

	//Descriptor Set Builder
    void AddToDescriptorLayoutCollection(std::vector<SingleDescriptorSetLayoutInfo>& desinfo,
        uint32_t binding, VkDescriptorType desType, VkShaderStageFlags stageFlags, uint32_t count);    
    void AddToDescrisptorLayoutWrite(std::vector<DescriptorWriteInfo>& desinfo,
        uint32_t binding, VkDescriptorType desType, DescriptorWriteInfo::Destype type, VkShaderStageFlags stageFlags, uint32_t count, std::vector<Sampler2D> data);
    void AddToDescrisptorLayoutWrite(std::vector<DescriptorWriteInfo>& desinfo,
        uint32_t binding, VkDescriptorType desType, DescriptorWriteInfo::Destype type, VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> data);

	VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<SingleDescriptorSetLayoutInfo>& descriptorInfo);
	VkDescriptorPool CreateDescriptorPool(const std::vector<SingleDescriptorSetLayoutInfo>& descriptorInfo, uint32_t count);
	std::vector<VkDescriptorSet> AllocateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,size_t count = 0 );
    void WriteDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets,const std::vector<DescriptorWriteInfo>& writeInfo);

    // ECS Rendering
private:
    struct FrameContext
    {
        VkCommandBuffer CMDBuffer;
        VkRenderPass Renderpass;
        VkFramebuffer currentFrameBuffer;
        VkFence currentFrameFence;
        VkSemaphore waitSemaphores;
        VkSemaphore signalSemaphores;
        uint32_t targetFrameIndex;
        uint32_t inFlightIndex;
        VkExtent2D extent;
    };

    VulkanRenderer::FrameContext GetCurrentFrameContext();
    void CMDBeginRecord(const VkCommandBuffer&);
    void CMDBeginRenderPass(const VkCommandBuffer&, const VkRenderPassBeginInfo&);
    // proble this one needs bit of rework
    void CMDRecordPushConstant(const VkCommandBuffer&, const VkPipelineLayout&,const VkShaderStageFlagBits&  ,const ModelMatrixPushConst&);
    void CMDRecordBindPipeline(const VkCommandBuffer&, const VkPipeline&, const VkPipelineBindPoint&);
    void CMDRecordDescriptorSet(const VkCommandBuffer&, const VkPipelineLayout&, VkPipelineBindPoint flag, const VkDescriptorSet*, uint32_t fristSet = 0, uint32_t count = 1, uint32_t desOffset = 0, const uint32_t* DynamicOffset = nullptr);
    void CMDRecordBindIndexedMesh(const VkCommandBuffer&, const IndexedVertexBuffer&);
    void CMDRecordDrawIndexedMesh(const VkCommandBuffer&, const IndexedVertexBuffer&);
    void CMDRecordDrawTerrainIndex(const VkCommandBuffer&, const IndexedVertexBuffer&);

    void CMDEndRenderPass(const VkCommandBuffer&);
    void CMDEndRecord(const VkCommandBuffer&);
    void CMDImageBarrier(const VkCommandBuffer& cmd, const VkImage& image, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccess,
        VkAccessFlags dstAccess, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask,
        uint32_t baseMip = 0, uint32_t levelCount = 1, uint32_t baseLayer = 0, uint32_t layerCount = 1);

    void CMDSubmitGraphics(VkCommandBuffer* cmds, uint32_t cmd_count, VkFence fence = VK_NULL_HANDLE, VkPipelineStageFlags* stageFlags = nullptr, VkSemaphore* waitSema = nullptr, uint32_t wait_count = 0, VkSemaphore* readySema = nullptr, uint32_t ready_count = 0);
    void CMDPresent(uint32_t SwapImageindex, VkSemaphore* waitSema = nullptr, uint32_t wait_count = 0);

    struct ECSRenderer;

    //Global Descriptorset
    //Main Camera
    DescriptorSetInfo GlobalSet;    
    
    std::weak_ptr<Component> camera;


    //Creation Helper functions
    void CreateSampler(VkSampler&, VkFilter, VkSamplerAddressMode, VkBorderColor,VkBool32 = VK_FALSE,VkBool32 = VK_TRUE);
    void CreateRenderPass(VkRenderPass& renderpass, std::vector<VkAttachmentDescription> colorAD, std::optional<VkAttachmentDescription> depthAD = std::nullopt);
    void CreateFrameBuffer(std::vector<VkImageView> images,VkExtent2D size, VkRenderPass& pass, VkFramebuffer& frameBuffer);
    void CreateSemaphore(VkSemaphore& semaphore);
    void CreateFence(VkFence& fence);

	void DestroyRenderPass(VkRenderPass& renderpass);
    void DestroyFrameBuffer(VkFramebuffer& frameBuffer);
    void DestroySemaphore(VkSemaphore& semaphore);
    void DestroyFence(VkFence& fence);
	void DestroySampler(VkSampler& sampler);
	void DestroyImageView(VkImageView& imageView);
	void DestroyImage(VkImage& image, VkDeviceMemory& imageMemory);


    void CopyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t, VkImageAspectFlags, uint32_t, uint32_t, uint32_t);
    void CubeImageLayoutTransition(VkImage, VkImageLayout srcLay, VkImageLayout dtsLay, VkPipelineStageFlags srdFlag, VkPipelineStageFlags dtsFlag,VkAccessFlags srcAcss, VkAccessFlags dtsAcss);
	//Shadow Mapping
    struct GlobalShadowMappingInfo
    {
        //Rendering handles
        VkRenderPass RenderPass;
        VkSampler ShadowSampler;
		std::vector<Sampler2D> ShadowTextures2D;
        std::vector<VkFramebuffer> FrameBuffers;	
        //CommandBufferData CMDBuffers;
		DescriptorSetInfo DesSetInfo;
        std::vector<PipelineInfo> PipelineInfo;
        //config info
        uint16_t NumOFCascadeMaps;
        std::vector<VkExtent2D> Exents;
        VkFormat format;
        VkImageTiling tile;
        VkImageUsageFlags useFlag;
        VkImageAspectFlags aspectFlag;
        VkMemoryPropertyFlags propFlag;
        VkImageLayout initial;
        VkImageLayout final; 
  
    };

	GlobalShadowMappingInfo shadowMappingInfo;

    void CreateGlobalShadowMappingResources(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
        VkMemoryPropertyFlags properties, VkImageLayout initialLayout, VkImageLayout finalLayout);
	void DestroyShadowMappingResources();

    void CreateGlobalShadowPipelineResources(std::string vertFile, std::string fragFile , std::shared_ptr<Component> globaLight);

public:
    void RenderECS(const std::vector<std::shared_ptr<Component>>& drawlist);

    PipeLineConfig GetMainPassPipeLineConfig();
    VulkanRenderer::GlobalShadowMappingInfo GetShadowInfo() { return shadowMappingInfo; }

    void CreateGlobalRources(std::shared_ptr<Component> cameraActor);
    void DestroyGlobalResources();

    void CreateGlobalDescriptionSet(const std::vector<SingleDescriptorSetLayoutInfo>& LayOutInfo,const std::vector<DescriptorWriteInfo>& WriteInfo);
    void DestroyGlobalDescriptionSet();
    DescriptorSetInfo GetGlobalDescriptionSet() { return GlobalSet; }

    
    //probly should be removed
    PipelineInfo CreateGraphicsPipeline(std::vector <VkDescriptorSetLayout> descriptorSetLayout, const char* vertFile, const char* fragFile,
        const char* tessCtrlFile = nullptr, const char* tessEvalFile = nullptr, const char* geomFile = nullptr);

    PipelineInfo CreateGraphicsPipeline(std::vector <VkDescriptorSetLayout> descriptorSetLayout, PipeLineConfig config, std::optional<std::string> vertFile, std::optional<std::string> fragFile
    , std::optional<std::string> tessCtrlFile = std::nullopt, std::optional<std::string> tessEvalFile = std::nullopt, std::optional<std::string> geomFile = std::nullopt);

    //Skybox
    Sampler2D SkyBoxSampler(std::vector<std::string> paths);

    // TERRAIN STUFF
public:
	void CreateTerrainBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffers);
    void CreateTerrainVertexBuffer(const std::vector<Vertex>& vertices, IndexedVertexBuffer& outBuffer);
	void CreateTerrainIndexBuffer(const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffers);
    void RenderTerrainChunk(IndexedVertexBuffer& terrainBuffers, const ModelMatrixPushConst& transform, PipelineInfo& pipelineInfo, DescriptorSetInfo descriptorSet);
	PipelineInfo CreateTerrainPipeline(VkDescriptorSetLayout descriptorSetLayout); // hard coded shaders
    void DrawTerrain(IndexedVertexBuffer chunk);
	void DestroyTerrainVertexBuffers(IndexedVertexBuffer terrainBuffers);
  
};
#endif 

