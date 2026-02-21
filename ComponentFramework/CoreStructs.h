#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <Vector.h>
#include <VMath.h>
#include <MMath.h>
#include <Hash.h>
#include <optional>
using namespace MATH;

struct BufferMemory {
    VkBuffer bufferID;
    VkDeviceMemory bufferMemoryID;
    VkDeviceSize bufferMemoryLength;
};


struct IndexBuffer {
    VkBuffer indexBufferID;
    VkDeviceMemory indexBufferMemoryID;
    VkDeviceSize indexBufferLength;
};

struct VertexBuffer {
    VkBuffer vertBufferID;
    VkDeviceMemory vertBufferMemoryID;
    VkDeviceSize vertBufferLength;
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

struct GlobalLightData {
    Matrix4 projectionMatrix;
    Matrix4 viewMatrix;
    Vec4 ambient;
    Vec4 diffused;
    Vec4 specular;
    Vec4 direction;
};
struct OrthConfig {
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
};
struct PerspectiveConfig {
    float fovy;
    float aspectRatio;
    float near;
    float far;
};
struct LightConfig {
    Vec4 ambient;
    Vec4 diffused;
    Vec4 specular;
    float distance;
    Quaternion orientation;
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
struct PipeLineConfig {
enum BlendMode {OPAQUE,ALPHA,ADDITIVE};
    //renderpass
    VkRenderPass renderPass = VK_NULL_HANDLE;
    //viewport and scissors should match framebuffersize usally
    VkExtent2D viewPortsize;
    //for custom vertex info
    std::optional<VkPipelineVertexInputStateCreateInfo> vertexinfo = std::nullopt;
    // for input assembly
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //rasterizer
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkBool32 depthBias = VK_FALSE;
    float depthBiasConstantFactor = 0.0f;
    float depthBiasSlopeFactor = 0.0f;
    float depthBiasClamp = 0.0f;
    //depth
    VkBool32 depthTestEnable = VK_TRUE; //Should fragments be compared against the depth buffer?
    VkBool32 depthWriteEnable = VK_TRUE; //Should passing fragments update the depth buffer?
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS; //How do we compare incoming depth vs stored depth?
    //color
    bool Color = true;
    BlendMode blendMode = BlendMode::OPAQUE;
    //Renderpass and viewport must be set
};


struct SingleDescriptorSetLayoutInfo {
    uint32_t binding; 
    uint32_t descriptorCount;
    VkDescriptorType descriptorType;
    VkSampler* pImmutableSamplers;
    VkShaderStageFlags stageFlags; 
};

//
struct DescriptorWriteInfo {
    enum Destype {UBO,SSBO,SAMPLER,TEXTURE,ARRTEXTURE};
    Destype type;
    uint32_t binding;
	VkDescriptorType descriptorType;
    uint32_t descriptorCount;
    VkDeviceSize offset;
    std::vector<BufferMemory> bufferMem;
    std::vector<Sampler2D> samplers;
 };

// terrain structs
// maybe add chunks here

// TODO: Replace this with scotts vertex
struct TerrainVertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
};

struct TerrainChunkData { // shared terrain mesh topology data
	IndexedVertexBuffer vertexBuffer; // shared among all terrain chunks
    //VertexBuffer vertexBuffer;
    ModelMatrixPushConst transform; // unique per chunk
    bool isInitialized = false;
};
