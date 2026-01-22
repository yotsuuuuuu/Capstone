#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <Vector.h>
#include <VMath.h>
#include <MMath.h>
#include <Hash.h>
using namespace MATH;

struct BufferMemory {
    VkBuffer bufferID;
    VkDeviceMemory bufferMemoryID;
    VkDeviceSize bufferMemoryLength;
};

struct Sampler2D {
    VkImage image;
    VkDeviceMemory imageDeviceMemory;
    VkImageView imageView;
    VkSampler sampler;
};

struct IndexedVertexBuffer {
    VkBuffer vertBufferID;
    VkDeviceMemory vertBufferMemoryID;
    VkDeviceSize vertBufferLength;

    VkBuffer indexBufferID;
    VkDeviceMemory indexBufferMemoryID;
    VkDeviceSize indexBufferLength;
};

struct CameraData { 
    Matrix4 projectionMatrix;
    Matrix4 viewMatrix;
};

#define MAX_LIGHTS 4 
struct LightsData {
    Vec4 pos[MAX_LIGHTS];
    Vec4 diffuse[MAX_LIGHTS];
    Vec4 specular[MAX_LIGHTS];
    Vec4 ambient;
    uint32_t numLights = 0;
};



/// A 3x3 cannot be sent to the GPU data alignment issues. 
/// If I try to send a 3x3 yo GPU it will be bumped up to a 4x4. 
/// I can fake it by storing the 3x3 in an array of 3 Vec4s as 
/// I have mapped below. Vulkan and OpenGl are column centric - right hand rule
/// The real reason to do this is to make room in push constant for other data.
///	Vec4    0(x)	3(y)	6(z)    0(w)			
///	Vec4    1(x)	4(y)	7(z)    0(w)		
///	Vec4    2(x)	5(y)	9(z)    0(w)		
struct ModelMatrixPushConst { // adjust for ease of use in the shader vert file.
    Matrix4 modelMatrix;
    Matrix4 normalMatrix;
};


struct DescriptorSetInfo {
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSet;
};

struct PipelineInfo {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};

struct RenderData {
    VkPipeline pipeline;
    IndexedVertexBuffer mesh;
    DescriptorSetInfo descriptorSetInfo;
    ModelMatrixPushConst modelMatrixPushConst;
};

struct CommandBufferData {
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
};


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


struct ImGuiContex {
    uint32_t version = VK_API_VERSION_1_0;
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    uint32_t queueFamily = (uint32_t)-1;
    VkQueue queue;
    VkRenderPass mainPass;
    SDL_Window* window;
    uint32_t framesInFlight;
    uint32_t numberofSwapchains;
    VkExtent2D windowExtents;
    float main_scale;
};

struct SingleDescriptorInfoCollection {
    uint32_t binding; /// Set the binding point - this must be an unique uint
    uint32_t descriptorCount;
    VkDescriptorType descriptorType;
    VkSampler* pImmutableSamplers;
    VkShaderStageFlags stageFlags; /// 

    /// If it's a UBO use these 
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize range;

    /// OR, if it's a sampler use these
    VkImageLayout imageLayout;
    VkImageView imageView;
    VkSampler sampler;

    /// There must be a swapchain number of UBO data
    std::vector<BufferMemory> bufferMem;

    /// OR just the one sampler data
    Sampler2D* pImageMem;
};

struct SingleDescriptorSetLayoutInfo {
    uint32_t binding; 
    uint32_t descriptorCount;
    VkDescriptorType descriptorType;
    VkSampler* pImmutableSamplers;
    VkShaderStageFlags stageFlags; 
};

struct DescriptorWriteInfo {
    uint32_t binding;
	VkDescriptorType descriptorType;
    uint32_t descriptorCount;
    VkDeviceSize offset;
    std::vector<BufferMemory> bufferMem;
    Sampler2D* pImageMem;
};
