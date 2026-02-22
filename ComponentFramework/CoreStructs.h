#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <Vector.h>
#include <VMath.h>
#include <MMath.h>
#include <Hash.h>
#include <optional>
#include <array>
using namespace MATH;

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

#define MAX_SHADOWS 3
struct GlobalLightData {
    Matrix4 projectionMatrix[MAX_SHADOWS];
    Matrix4 viewMatrix[MAX_SHADOWS];
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
    enum Destype {STATIC_UBO,STATIC_SAMPLER,STATIC_SSBO,PER_FRAME_UBO,
                PER_FRAME_SAMPLER, PER_FRAME_ARR_SAMPLER,STATIC_ARR_SAMPLER};
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
	ModelMatrixPushConst transform; // unique per chunk
    bool isInitialized = false;
};
