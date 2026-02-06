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
#include <array>
#include <queue>

#include "CoreStructs.h"
#include "Renderer.h"
#include "DescriptorSetBuilder.h"
#include "VkImGUISystem.h"
#include "Component.h"

#include <Vector.h>
#include <VMath.h>
#include <MMath.h>
#include <Hash.h>
#include <memory>
using namespace MATH;


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
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};



struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec2 texCoord;

    /// Used in vertex deduplication
    bool operator == (const Vertex& other) const {
        return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
    }   

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
       
}; 


/// Generate a hash of a Vertex, used in vertex deduplication
/// Adding this to namespace std is called a namespace injection
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const noexcept {
            size_t hash1 = hash<Vec3>()(vertex.pos);
            size_t hash2 = hash<Vec3>()(vertex.normal);
            size_t hash3 = hash<Vec2>()(vertex.texCoord);
            size_t result = ((hash1 ^ (hash2 << 1)) >> 1) ^ (hash3 << 1);
            return result;
        }
    };
}



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
   

    template<class T>
    std::vector<BufferMemory> CreateUniformBuffers() {
        std::vector<BufferMemory> ubo;
        VkDeviceSize bufferSize = sizeof(T);
        ubo.resize(numSwapchains);
        for (size_t i = 0; i < numSwapchains; i++) {
            ubo[i].bufferMemoryLength = bufferSize;
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                ubo[i].bufferID, ubo[i].bufferMemoryID);
        }
        return ubo;
    }

    template <class T>
    void UpdateUniformBuffer(const T& srcData, const std::vector<BufferMemory> bufferMemory) {
        void* data;
        int num = static_cast<int>(numSwapchains);
        for (int i = 0; i < num; ++i) {
            vkMapMemory(device, bufferMemory[i].bufferMemoryID, 0, sizeof(T), 0, &data);
            memcpy(data, &srcData, bufferMemory[i].bufferMemoryLength);
            vkUnmapMemory(device, bufferMemory[i].bufferMemoryID);
        }
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
    void createImageViews();
    
   

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
    void AddToDescrisptorLayoutCollection(std::vector<SingleDescriptorSetLayoutInfo>& desinfo,
        uint32_t binding, VkDescriptorType desType, VkShaderStageFlags stageFlags, uint32_t count);
    void AddToDescrisptorLayoutWrite(std::vector<DescriptorWriteInfo>& desinfo,
        uint32_t binding, VkDescriptorType desType, VkShaderStageFlags stageFlags, uint32_t count,Sampler2D* data);
    void AddToDescrisptorLayoutWrite(std::vector<DescriptorWriteInfo>& desinfo,
        uint32_t binding, VkDescriptorType desType, VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> data);

	VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<SingleDescriptorSetLayoutInfo>& descriptorInfo);
	VkDescriptorPool CreateDescriptorPool(const std::vector<SingleDescriptorSetLayoutInfo>& descriptorInfo, uint32_t count);
	std::vector<VkDescriptorSet> AllocateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
    void WriteDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets,const std::vector<DescriptorWriteInfo>& writeInfo);

    // ECS Rendering
private:
    struct FrameContext
    {
        VkSemaphore waitSemaphores;
        VkSemaphore signalSemaphores;
        VkFence currentframeFence;
        VkCommandBuffer CMDBuffer;
        VkRenderPass Renderpass;
        uint32_t targetFrameIndex;
        uint32_t currentFrameIndex;
        VkFramebuffer currentFrameBuffer;
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
    void CMDEndRenderPass(const VkCommandBuffer&);
    void CMDEndRecord(const VkCommandBuffer&);
    void CMDMemoryBarrier(const VkCommandBuffer&, std::optional<VkMemoryBarrier> = std::nullopt);

    void CMDSubmitGraphics(VkCommandBuffer* cmds, uint32_t cmd_count, VkFence fence = VK_NULL_HANDLE, VkPipelineStageFlags* stageFlags = nullptr, VkSemaphore* waitSema = nullptr, uint32_t wait_count = 0, VkSemaphore* readySema = nullptr, uint32_t ready_count = 0);
    void CMDPresent(uint32_t SwapImageindex, VkSemaphore* waitSema = nullptr, uint32_t wait_count = 0);

    struct ECSRenderer;
public:
    void RenderECS(const std::vector<Ref<Component>>& drawlist);

    //Global Descriptorset
private:
    DescriptorSetInfo GlobalSet;

public:
    void CreateGlobalDescriptionSet(const std::vector<SingleDescriptorSetLayoutInfo>& LayOutInfo,const std::vector<DescriptorWriteInfo>& WriteInfo);
    void DestroyGlobalDescriptionSet();
    DescriptorSetInfo GetGlobalDescriptionSet() { return GlobalSet; }

    void BindDescriptorSet(VkPipelineLayout pipelineLayout, const std::vector<VkDescriptorSet> descriptorSet,uint32_t setID);
    
    PipelineInfo CreateGraphicsPipeline(std::vector <VkDescriptorSetLayout> descriptorSetLayout, const char* vertFile, const char* fragFile,
        const char* tessCtrlFile = nullptr, const char* tessEvalFile = nullptr, const char* geomFile = nullptr);
    PipelineInfo CreateGraphicsPipeline(std::vector <VkDescriptorSetLayout> descriptorSetLayout, PipeLineConfig config, std::optional<std::string> vertFile, std::optional<std::string> fragFile
    , std::optional<std::string> tessCtrlFile = std::nullopt, std::optional<std::string> tessEvalFile = std::nullopt, std::optional<std::string> geomFile = std::nullopt);
private:
    //Creation Helper functions
    void CreateSampler(VkSampler&, VkFilter, VkSamplerAddressMode, VkBorderColor,VkBool32 = VK_FALSE);
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


	//Shadow Mapping
    struct GlobalShadowMappingInfo
    {
        //Rendering handles
        VkRenderPass RenderPass;
		std::vector<Sampler2D> ShadowTextures2D;
        std::vector<VkFramebuffer> FrameBuffers;
		std::vector<VkSemaphore> WaitSemaphores;// 2 for the number of frames in flight
		std::vector<VkSemaphore> SignalSemaphores;// 2
		std::vector<VkFence> InFlightFences; //2 
        //CommandBufferData CMDBuffers;
		DescriptorSetInfo DesSetInfo;
        PipelineInfo PipelineInfo;
        //config info
        VkExtent2D Exents;
        VkFormat format;
        VkImageTiling tile;
        VkImageUsageFlags useFlag;
        VkImageAspectFlags aspectFlag;
        VkMemoryPropertyFlags propFlag;
        VkImageLayout initial;
        VkImageLayout final; 
        //temp variables
        std::vector<BufferMemory> LightsUBO;
    };

	GlobalShadowMappingInfo shadowMappingInfo;

    void CreateGlobalShadowMappingResources(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
        VkMemoryPropertyFlags properties, VkImageLayout initialLayout, VkImageLayout finalLayout);

    void CreateGlobalShadowPipelineResources(std::string vertFile, std::string fragFile , Ref<Component> globaLight);

	void DestroyShadowMappingResources();
public:


    GlobalShadowMappingInfo GetShadowMappingInfo() { return shadowMappingInfo; }

    //Temp function just to initilize the variables
    void CreateGlobalRources(const std::vector<BufferMemory>& cameraUBO);
    void DestroyGlobalResources();




    // TERRAIN STUFF
public:
	void CreateTerrainBuffers(const std::vector<TerrainVertex>& vertices, const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffers);
    void CreateTerrainVertexBuffer(const std::vector<TerrainVertex>& vertices, IndexedVertexBuffer& outBuffer);
	void CreateTerrainIndexBuffer(const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffers);
    void RenderTerrainChunk(IndexedVertexBuffer& terrainBuffers, const ModelMatrixPushConst& transform, PipelineInfo& pipelineInfo, DescriptorSetInfo descriptorSet);
	PipelineInfo CreateTerrainPipeline(VkDescriptorSetLayout descriptorSetLayout); // hard coded shaders
    void DrawTerrain(IndexedVertexBuffer chunk);

};
#endif 

