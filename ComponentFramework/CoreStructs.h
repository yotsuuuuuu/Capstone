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

// SSBO Padding for lights
struct alignas(16) CLightData {
    Vec4 position_radius;
    Vec4 colour_intensity;
    Vec4 direction_inner;
    Vec4 outer_type_emissiveScale_audioID;
};

struct  Cluster
{
    Vec4 minPoint;
    Vec4 maxPoint;
    uint32_t count;
    uint32_t pad[3];
    uint32_t lightIndices[100];
};

struct alignas(16) SYS_LIGHT_DATA { // This needs to match the shader
    Matrix4 inverseProjection;      //64 : 64
    uint32_t gridSize[3];           // 12 : 76
    uint32_t _pad1;                 // 4  : 80
    uint32_t screenDimensions[2];   // 8 : 88
    float zPlanes[2];              // 8  : 96 
    uint32_t lightCount;            // 4  : 100
    uint32_t clusterCount;          // 4  : 104
    uint32_t _pad2[2];              // 8  : 112 
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

    bool dynamicViewport = false;
    //Renderpass and viewport must be set if not dynamic
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

struct AABB
{
    Vec3 min;
    Vec3 max;
};

struct TerrainChunkData { // shared terrain mesh topology data
	IndexedVertexBuffer vertexBuffer; // shared among all terrain chunks
	ModelMatrixPushConst transform; // unique per chunk
	std::vector<Vertex> vertices; // unique per chunk, used for collision and culling
    bool isInitialized = false;
	bool isCulled = false;
	Vec2 chunkPos; // x,z position in world space
	AABB aabb;
};

struct AudioBands
{
    float sub = 0.0f;       // 20 Hz    -  60 Hz
    float bass = 0.0f;      // 61 Hz    - 130 Hz
    float highBass = 0.0f;  // 131 Hz   - 262 Hz
    float lowMid = 0.0f;    // 263 Hz   - 523 Hz
    float midMid = 0.0f;    // 524 Hz   - 1046 Hz
    float highMid = 0.0f;   // 1047 Hz  - 2093 Hz
    float lowHigh = 0.0f;   // 2094 Hz  - 4186 Hz
    float midHigh = 0.0f;   // 4187 Hz  - 8000 Hz
    float highHigh = 0.0f;  // 8000 Hz  - 12000 Hz
    float air = 0.0f;       // 12001 Hz - 20000 Hz
};

struct alignas(16) AudioBandsUBO {
    Matrix4 bands;
};

enum class AudioState
{
	PLAY,
	PAUSE,
    NEXT,
	PREV,
	STOP
};


struct RenderTarget {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkFormat format;
    VkExtent2D extent;
    Sampler2D MakeSampler(VkSampler sampler) {
        Sampler2D s{};
        s.image = image;
        s.imageView = view;
        s.imageDeviceMemory = memory;
        s.sampler = sampler;
        return s;
    }
};

struct HDRContext {

    VkSampler sampler;

    // hdr per frames in flight
    VkRenderPass hdrRenderPass;
    std::vector<RenderTarget> hdrColor; // color images
    std::vector<RenderTarget> hdrDepth; // depth iamges
    std::vector<VkFramebuffer> hdrFramebuffers; // frame buffer

    //Bloom 
    VkRenderPass bloomDonwPass;
    VkRenderPass bloomUpPass;
    std::vector<RenderTarget> bloomMips; // numberinflight * number of mips
    std::vector<VkFramebuffer> bloomDownFramebuffers;
    std::vector<VkFramebuffer> bloomUpFramebuffers;
    

    //VkRenderPass tonemapRenderPass; Use the Main Render Pass
    
    PipelineInfo tonePassPipeline;    
    DescriptorSetInfo tonemapDescriptors;

    PipelineInfo thresholdPipeline;
    PipelineInfo donwSamplePipeline;
    PipelineInfo upSamplePipeline;
    DescriptorSetInfo bloomDescriptors;

    uint32_t bloomMipLevels = 5;
    float bloomThreshold = 1.0f;
    float bloomStrength = 1.0f;
};

struct BloomPush {
    float bloomThreshold;
    float bloomStrength;
};

struct GlobalShadowMappingInfo
{
    //Rendering handles
    VkRenderPass RenderPass;
    VkSampler ShadowSampler;
    std::vector<Sampler2D> ShadowTextures2D;
    std::vector<VkFramebuffer> FrameBuffers;
    //for rendering 
    VkCommandPool CMDpool;
    std::vector<VkCommandBuffer> CMDBuffers;
    DescriptorSetInfo DesSetInfo;
    std::vector<PipelineInfo> PipelineInfo;
    //sync objects
    std::vector<VkSemaphore> ReadySignals;
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

struct FrameContext
{
    VkCommandBuffer CMDBuffer;
    VkRenderPass SwapChainRenderpass;
    VkRenderPass HDRRenderPass;
    VkFramebuffer currentSwapChainFrameBuffer;
    VkFramebuffer currentHDRFrameBuffer;
    VkFence currentFrameFence;
    VkSemaphore waitSemaphores;
    VkSemaphore signalSemaphores;
    uint32_t targetFrameIndex;
    uint32_t inFlightIndex;
    VkExtent2D extent;
};

struct SkyboxPush {
    Vec4  ColorTint;
    float Bloomfactor = 1.0f;
};


struct SystemsUBOs {
    BufferMemory AudioGPUData;
    BufferMemory EngineData; // unsure what to put here
    BufferMemory TerrainData;
};

struct SongTime
{
    unsigned int min = 0;
    unsigned int sec = 0;
};

struct TerraindataUBO {
    Vec4 max_min_lineWidth_pad;
    Vec4 fadeStart_fadeEnd_gridScaleX_gridScaleY;
    Vec4 maxColor;
	Vec4 minColor;
};