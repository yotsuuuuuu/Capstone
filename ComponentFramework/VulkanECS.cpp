#include "VulkanRenderer.h"
#include "CActor.h"
#include "CCameraActor.h"
#include "CCamera.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CMesh.h"
#include "CGlobalLight.h"
#include "CTransform.h"
#include "CWorld.h"
#include "CSkyBox.h"
#include "EngineContext.h"
#include "SYS_Light.h"
#include "AssetManager.h"
#include "VkImGUISystem.h"
#include "imgui.h"
#include <unordered_map>

void VulkanRenderer::CreateGlobalDescriptionSet(const std::vector<SingleDescriptorSetLayoutInfo>& LayOutInfo, const std::vector<DescriptorWriteInfo>& WriteInfo)
{
    GlobalSet.descriptorSetLayout = CreateDescriptorSetLayout(LayOutInfo);
    GlobalSet.descriptorPool = CreateDescriptorPool(LayOutInfo, 2);
    GlobalSet.descriptorSet = AllocateDescriptorSets(GlobalSet.descriptorPool, GlobalSet.descriptorSetLayout);
    WriteDescriptorSets(GlobalSet.descriptorSet, WriteInfo);
}

void VulkanRenderer::DestroyGlobalDescriptionSet()
{
    DestroyDescriptorSet(GlobalSet);
}


bool VulkanRenderer::CreateGlobalRources(EngineContext& Ecntx)
{
  
    auto MainCamera = Ecntx.assetManager->GetCamera();
    if (!MainCamera) {
        Debug::FatalError("CAMERA NOT SET", __FILE__, __LINE__);
        return false;
    }
    camera = MainCamera;

    auto CameraActor = std::dynamic_pointer_cast<CActor>(MainCamera);
    if (!CameraActor) {
        Debug::FatalError("NO VALID ACTOR", __FILE__, __LINE__);
        return false;
    }

    auto Glight = CameraActor->GetComponent<CGlobalLight>();
    if (!Glight) {
        Debug::FatalError("NO VALID GLOBAL LIGHT COMPONENT", __FILE__, __LINE__);
        return false;
    }

    auto Camera = CameraActor->GetComponent<CCamera>();
    if (!Glight) {
        Debug::FatalError("NO VALID CAMERA COMPONENT", __FILE__, __LINE__);
        return false;
    }

    uint32_t shadowmapsize = SHAWDOW_SIZE * 1;
    // create the shadow resources
    CreateGlobalShadowMappingResources(shadowmapsize, shadowmapsize, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    CreateGlobalShadowPipelineResources("./shaders/GlobalLight.vert.spv", "./shaders/GlobalLight.frag.spv", Glight);

    // then create global resources
    //TODO: ADD THE LIGTH SYSTEM SSBOs TO THE GLOBAL SET
    std::vector<SingleDescriptorSetLayoutInfo> layoutGlobal;
    AddToDescriptorLayoutCollection(layoutGlobal, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    AddToDescriptorLayoutCollection(layoutGlobal, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    AddToDescriptorLayoutCollection(layoutGlobal, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    AddToDescriptorLayoutCollection(layoutGlobal, 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    AddToDescriptorLayoutCollection(layoutGlobal, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    AddToDescriptorLayoutCollection(layoutGlobal, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

    std::vector<DescriptorWriteInfo> writeGlobal;
    AddToDescrisptorLayoutWrite(writeGlobal, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::PER_FRAME_UBO, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, Camera->GetCameraUBO());
    AddToDescrisptorLayoutWrite(writeGlobal, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::PER_FRAME_UBO, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, Glight->GetMainUBO());
    AddToDescrisptorLayoutWrite(writeGlobal, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorWriteInfo::Destype::PER_FRAME_ARR_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, shadowMappingInfo.ShadowTextures2D);
    AddToDescrisptorLayoutWrite(writeGlobal, 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::STATIC_UBO, VK_SHADER_STAGE_FRAGMENT_BIT, 1, Ecntx.lightSys->GetSysUBO());
    AddToDescrisptorLayoutWrite(writeGlobal, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DescriptorWriteInfo::Destype::STATIC_SSBO, VK_SHADER_STAGE_FRAGMENT_BIT, 1, Ecntx.lightSys->GetClusterSSBO());
    AddToDescrisptorLayoutWrite(writeGlobal, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DescriptorWriteInfo::Destype::STATIC_SSBO, VK_SHADER_STAGE_FRAGMENT_BIT, 1, Ecntx.lightSys->GetLightSSBO());
    CreateGlobalDescriptionSet(layoutGlobal, writeGlobal);

    //
    //add the skybox to the main camera actor
    std::vector<std::string> skyboxFiles = { "./textures/skybox/px.png","./textures/skybox/nx.png",
                                             "./textures/skybox/py.png","./textures/skybox/ny.png",
                                             "./textures/skybox/pz.png","./textures/skybox/nz.png" };
    Ref<CSkyBox> sky = std::make_shared<CSkyBox>(nullptr, this, skyboxFiles);
    if (!sky->OnCreate()) {
        Debug::FatalError("FAILED TO CREATE SKYBOX", __FILE__, __LINE__);
        return false;
    }
    CameraActor->AddComponent<CSkyBox>(sky);

    return true;
}

void VulkanRenderer::DestroyGlobalResources()
{
    vkDeviceWaitIdle(device);
    DestroyGlobalDescriptionSet();
    DestroyShadowMappingResources();
}

VulkanRenderer::FrameContext VulkanRenderer::GetCurrentFrameContext()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    FrameContext context{};
    context.targetFrameIndex = imageIndex;
    context.inFlightIndex = currentFrame;
    context.CMDBuffer = primaryCommandBuffer.commandBuffers[imageIndex];
    context.currentFrameFence = inFlightFences[currentFrame];
    context.waitSemaphores = imageAvailableSemaphores[currentFrame];
    context.signalSemaphores = renderFinishedSemaphores[currentFrame];
    context.currentFrameBuffer = swapChainFramebuffers[imageIndex];
    context.Renderpass = renderPass;
    context.extent = swapChainExtent;

    return context;
   
}

void VulkanRenderer::CreateRenderPass(VkRenderPass& renderpass_, std::vector<VkAttachmentDescription> colorAD, std::optional<VkAttachmentDescription> depthAD)
{
    //VkAttachmentDescription colorAttachment{};
    //colorAttachment.format = swapChainImageFormat;
    //colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    
	uint32_t ColorAttachmentCount = static_cast<uint32_t>(colorAD.size());
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = ColorAttachmentCount;
	


    std::vector<VkAttachmentReference> ColorAttachmentsRefeneces;
    VkAttachmentReference depthAttachmentRef{};
    for (uint32_t i = 0; i < ColorAttachmentCount; i++) {
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = i;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		ColorAttachmentsRefeneces.push_back(colorAttachmentRef);

    }
    if (depthAD) {
		depthAttachmentRef.attachment = ColorAttachmentCount;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
		colorAD.push_back(depthAD.value());
    }
    else {
        subpass.pDepthStencilAttachment = nullptr;
    }
      
   
    subpass.pColorAttachments = ColorAttachmentsRefeneces.data();

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(colorAD.size());
    renderPassInfo.pAttachments = colorAD.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpass_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanRenderer::CreateFrameBuffer(std::vector<VkImageView> images, VkExtent2D size, VkRenderPass& pass, VkFramebuffer& frameBuffer)
{
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(images.size());
    framebufferInfo.pAttachments = images.data();
    framebufferInfo.width = size.width;
    framebufferInfo.height = size.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void VulkanRenderer::CreateSemaphore(VkSemaphore& semaphore)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Semaphore object");
    }
}

void VulkanRenderer::CreateFence(VkFence& fence)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence");
    }
}

VkCommandPool VulkanRenderer::CreateCMDPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{

    VkCommandPool cmd = VK_NULL_HANDLE; 

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = flags;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &cmd) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    return cmd;
}

VkCommandBuffer VulkanRenderer::AllocatedCMDbuffer(const VkCommandPool& cmd, VkCommandBufferLevel level)
{
    VkCommandBuffer CMDBUFFER;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmd;
    allocInfo.level = level;
    allocInfo.commandBufferCount = (uint32_t)(1);

    if (vkAllocateCommandBuffers(device, &allocInfo, &CMDBUFFER) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    return CMDBUFFER;

}

std::vector<VkCommandBuffer> VulkanRenderer::AllocatedCMDbuffer(const VkCommandPool& cmd, VkCommandBufferLevel level,  uint32_t count)
{

    std::vector<VkCommandBuffer> CMDBUFFER;
    CMDBUFFER.resize(count);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmd;
    allocInfo.level = level;
    allocInfo.commandBufferCount = count;

    if (vkAllocateCommandBuffers(device, &allocInfo, CMDBUFFER.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    return CMDBUFFER;
   
}

void VulkanRenderer::DestroyCommandPool(VkCommandPool& cmdPool)
{
    if (cmdPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, cmdPool, nullptr);
        cmdPool = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::DestroyCommandBuffer(std::vector<VkCommandBuffer>& CMDBuffer, const VkCommandPool& pool)
{
    if (!CMDBuffer.empty()) {

        vkFreeCommandBuffers(device,
            pool,
            static_cast<uint32_t>(CMDBuffer.size()),
            CMDBuffer.data());

        CMDBuffer.clear();
    }
}

void VulkanRenderer::DestroyRenderPass(VkRenderPass& renderpass)
{
    if (renderpass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device, renderpass, nullptr);
    }
}

void VulkanRenderer::DestroyFrameBuffer(VkFramebuffer& frameBuffer)
{
    if (frameBuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }
}

void VulkanRenderer::DestroySemaphore(VkSemaphore& semaphore)
{
    if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
}

void VulkanRenderer::DestroyFence(VkFence& fence)
{
    if (fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, fence, nullptr);
    }
}

void VulkanRenderer::DestroySampler(VkSampler& sampler)
{
    if (sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, sampler, nullptr);
    }
}


void VulkanRenderer::DestroyImageView(VkImageView& imageView)
{
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, imageView, nullptr);
    }
}

void VulkanRenderer::DestroyImage(VkImage& image, VkDeviceMemory& imageMemory)
{
    if (imageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, imageMemory, nullptr);
        vkDestroyImage(device, image, nullptr);
    }
}

void VulkanRenderer::CMDBeginRecord(const VkCommandBuffer& cmd)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void VulkanRenderer::CMDBeginRenderPass(const VkCommandBuffer& cmd, const VkRenderPassBeginInfo& renderInfo)
{
    vkCmdBeginRenderPass(cmd, &renderInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::CMDRecordPushConstant(const VkCommandBuffer& cmd, const VkPipelineLayout& layout, 
    const VkShaderStageFlagBits& flag, const ModelMatrixPushConst& push)
{
    vkCmdPushConstants(cmd, layout, flag, 0, sizeof(ModelMatrixPushConst), &push);
}

void VulkanRenderer::CMDRecordBindPipeline(const VkCommandBuffer& cmd, const VkPipeline& pipeline, const VkPipelineBindPoint& flag)
{
    vkCmdBindPipeline(cmd, flag, pipeline);
}


void VulkanRenderer::CMDRecordDescriptorSet(const VkCommandBuffer& cmd, const VkPipelineLayout& layout, 
    VkPipelineBindPoint flag, const VkDescriptorSet* DesSet, uint32_t fristSet, uint32_t count, uint32_t desOffset, const uint32_t* DynamicOffset)
{
    vkCmdBindDescriptorSets(cmd, flag, layout, fristSet, count, DesSet, desOffset, DynamicOffset);
}

void VulkanRenderer::CMDRecordBindIndexedMesh(const VkCommandBuffer& cmd, const IndexedVertexBuffer& mesh)
{
    VkBuffer vertexBuffers[] = { mesh.vertBufferID };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, mesh.indexBufferID, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanRenderer::CMDRecordDrawIndexedMesh(const VkCommandBuffer& cmd, const IndexedVertexBuffer& mesh)
{
    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indexBufferLength), 1, 0, 0, 0);
}


void VulkanRenderer::CMDEndRenderPass(const VkCommandBuffer& cmd)
{
    vkCmdEndRenderPass(cmd);
}

void VulkanRenderer::CMDEndRecord(const VkCommandBuffer& cmd)
{
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanRenderer::CMDRecordDistpatch(const VkCommandBuffer& cmd, uint32_t groupX, uint32_t groupY, uint32_t groupz)
{
    vkCmdDispatch(cmd, groupX, groupY, groupz);
}


void VulkanRenderer::CMDImageBarrier(const VkCommandBuffer& cmd, const VkImage& image, VkPipelineStageFlags srcStage,
    VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
    VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, 
    uint32_t baseMip, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.layerCount = layerCount;
    barrier.subresourceRange.baseArrayLayer = baseLayer;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMip;
    barrier.subresourceRange.levelCount = levelCount;

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

}

void VulkanRenderer::CMDSubmitComputeQueue(VkCommandBuffer* cmds, uint32_t cmd_count, VkFence fence, VkPipelineStageFlags* stageFlags, VkSemaphore* waitSema, uint32_t wait_count, VkSemaphore* readySema, uint32_t ready_count)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = wait_count;
    submitInfo.pWaitSemaphores = waitSema;
    submitInfo.pWaitDstStageMask = stageFlags;

    submitInfo.commandBufferCount = cmd_count;
    submitInfo.pCommandBuffers = cmds;

    submitInfo.signalSemaphoreCount = ready_count;
    submitInfo.pSignalSemaphores = readySema;

    if (vkQueueSubmit(computeQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}


void VulkanRenderer::CMDSubmitGraphicsQueue(VkCommandBuffer* cmds, uint32_t cmd_count, VkFence fence,
    VkPipelineStageFlags* stageFlags, VkSemaphore* waitSema, uint32_t wait_count, VkSemaphore* readySema, uint32_t ready_count)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = wait_count;
    submitInfo.pWaitSemaphores = waitSema;
    submitInfo.pWaitDstStageMask = stageFlags;

    submitInfo.commandBufferCount = cmd_count;
    submitInfo.pCommandBuffers = cmds;

    submitInfo.signalSemaphoreCount = ready_count;
    submitInfo.pSignalSemaphores = readySema;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void VulkanRenderer::CMDPresent(uint32_t SwapImageindex, VkSemaphore* waitSema, uint32_t wait_count)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = wait_count;
    presentInfo.pWaitSemaphores = waitSema;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &SwapImageindex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

struct VulkanRenderer::ECSRenderer {
private:
    struct DrawItem{
        PipelineInfo pipeInfo;
        VkDescriptorSet set;
        uint32_t setID;
        IndexedVertexBuffer mesh;
        ModelMatrixPushConst push;
    };
public:
    static void Render(VulkanRenderer* VKRNDR, const EngineContext& Ecntx, const std::vector<std::shared_ptr<Component>>& drawlist) {
        //  Passes i want to do currently doing forward rendering
        //  Sky light shadow pass : the Main light source that affects the scene
        //  Normal forward pass
        //  Post process bloom pass
        //  ImGUI 
      
      
        // 1 Get current render frame info
        VulkanRenderer::FrameContext framecntx =  VKRNDR->GetCurrentFrameContext();


        VulkanRenderer::GlobalShadowMappingInfo shadowcntx = VKRNDR->GetShadowInfo();
        // 1.1.1 UPDATE PER FRAME UBO
        if (auto cam = VKRNDR->camera.lock()) {
            auto MainCamera = std::dynamic_pointer_cast<CActor>(cam);
            //context get data form fmdo  then update buffer.
            MainCamera->GetComponent<CCamera>()->UpdateUBO(framecntx.inFlightIndex);
            MainCamera->GetComponent<CGlobalLight>()->UpdateUBO(framecntx.inFlightIndex);
        }
        
        Ecntx.lightSys->ComputeLightClusters(framecntx.inFlightIndex);

        VkPipelineLayout line;
        // 1.1 get draw items
        std::unordered_map<VkPipeline, std::vector<DrawItem>> DrawingBuckets;      
        for (const auto& comp : drawlist) {
            Ref<CActor> a = std::dynamic_pointer_cast<CActor>(comp);
            if (a) {
                auto mat = a->GetComponent<CMaterial>();
                auto mesh = a->GetComponent<CMesh>();
                // sohuld check for transform
                if (mat && mesh) {
                    DrawItem item = GetDrawItem(a, framecntx);
                    // 1.2 sort them into buckets
                    DrawingBuckets[item.pipeInfo.pipeline].push_back(item);
                    line = item.pipeInfo.pipelineLayout;
                }       
                auto world = a->GetComponent<CWorld>();
                if (world && mat) {
                    PipelineInfo info = mat->GetPipelineInfo();
                    VkDescriptorSet set = mat->GetDescriptorSet()[framecntx.inFlightIndex];
                    auto chunkMap = world->GetChunkRenderData();
                    for (const auto& pair : *chunkMap) {
                        if (pair.second.isCulled) {
							//std::cout << "culled chunk at " << pair.first.x << "," << pair.first.y << "," << pair.first.z << std::endl;
                            continue;
						}
                        DrawItem item;
                        item.mesh = pair.second.vertexBuffer;
                        item.push = pair.second.transform;
                        item.setID = mat->GetSetValue();
                        item.pipeInfo = info;
                        item.set = set;
                        DrawingBuckets[item.pipeInfo.pipeline].push_back(item);
                    }
                }

            }
        }
        //1.3 skybox draw item for main pass
        DrawItem skybox;
        if (auto cam = VKRNDR->camera.lock()) {
           auto MainCamera =  std::dynamic_pointer_cast<CActor>(cam);
           auto CskyBox = MainCamera->GetComponent<CSkyBox>();
           skybox.mesh = CskyBox->GetMesh();
           skybox.pipeInfo = CskyBox->GetPipeline();
           skybox.set = CskyBox->GetSet()[framecntx.inFlightIndex];
           skybox.setID = 1;
        }
        else {
            throw std::runtime_error("Main Camera is in valid");
        }
        // 3 Start recording
        VKRNDR->CMDBeginRecord(framecntx.CMDBuffer);
        {// Shadow Pass

            auto cachedLayout = shadowcntx.PipelineInfo[0].pipelineLayout;
            std::array<VkClearValue, 1> clearValue{};
            clearValue[0].depthStencil = { 1.0f,0 };

            for (uint32_t i = 0; i < shadowcntx.NumOFCascadeMaps; i++) { // For each shadow map need it for the 
                // flat array so my row is the current in flight index my colums are i with a witdh of number of cascademaps
                // this is used to acess the 3 resoultion cascade maps high med and low.
                size_t index = framecntx.inFlightIndex * shadowcntx.NumOFCascadeMaps + i;
                VkRenderPassBeginInfo renderinfo{};
                renderinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderinfo.renderPass = shadowcntx.RenderPass;
                renderinfo.framebuffer = shadowcntx.FrameBuffers[index];
                renderinfo.renderArea.offset = { 0,0 };
                // there are number of exents that match the number of CascadeMaps
                renderinfo.renderArea.extent = shadowcntx.Exents[i];
                renderinfo.clearValueCount = static_cast<uint32_t>(clearValue.size());
                renderinfo.pClearValues = clearValue.data();
                VKRNDR->CMDBeginRenderPass(framecntx.CMDBuffer, renderinfo);
                // there are number of pipeline that match the number of CascadeMaps
                VKRNDR->CMDRecordBindPipeline(framecntx.CMDBuffer, shadowcntx.PipelineInfo[i].pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
                VKRNDR->CMDRecordDescriptorSet(framecntx.CMDBuffer, cachedLayout, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    &shadowcntx.DesSetInfo.descriptorSet[index]);

                for (const auto& pair : DrawingBuckets) {
                    for (const auto& item : pair.second) {
                        VKRNDR->CMDRecordBindIndexedMesh(framecntx.CMDBuffer, item.mesh);
                        VKRNDR->CMDRecordPushConstant(framecntx.CMDBuffer, cachedLayout, VK_SHADER_STAGE_VERTEX_BIT, item.push);
                        VKRNDR->CMDRecordDrawIndexedMesh(framecntx.CMDBuffer, item.mesh);
                    }
                }

                VKRNDR->CMDEndRenderPass(framecntx.CMDBuffer);
            }

        }
   
        // An image must leave a render pass in the layout it will be used in next.
        // If the next use requires a different layout, a memory image barrier must be used to transition it.
       

        { // the main pass
            VkRenderPassBeginInfo renderinfo{};
            renderinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderinfo.renderPass = framecntx.Renderpass;
            renderinfo.framebuffer = framecntx.currentFrameBuffer;
            renderinfo.renderArea.offset = { 0, 0 };
            renderinfo.renderArea.extent = framecntx.extent;
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };
            renderinfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderinfo.pClearValues = clearValues.data();
            VKRNDR->CMDBeginRenderPass(framecntx.CMDBuffer, renderinfo);
            //global discriptor bind
            auto globalset = VKRNDR->GetGlobalDescriptionSet();
            VKRNDR->CMDRecordDescriptorSet(framecntx.CMDBuffer, skybox.pipeInfo.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, &globalset.descriptorSet[framecntx.inFlightIndex]);
            // draw sky box
            VKRNDR->CMDRecordBindPipeline(framecntx.CMDBuffer, skybox.pipeInfo.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
            VKRNDR->CMDRecordDescriptorSet(framecntx.CMDBuffer, skybox.pipeInfo.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, &skybox.set, skybox.setID);
            VKRNDR->CMDRecordBindIndexedMesh(framecntx.CMDBuffer, skybox.mesh);
            VKRNDR->CMDRecordDrawIndexedMesh(framecntx.CMDBuffer, skybox.mesh);
            // draw the rest of the items
            for (const auto& pair : DrawingBuckets) {
                VKRNDR->CMDRecordBindPipeline(framecntx.CMDBuffer, pair.first, VK_PIPELINE_BIND_POINT_GRAPHICS);
                for (const auto& item : pair.second) {
                    VKRNDR->CMDRecordDescriptorSet(framecntx.CMDBuffer, item.pipeInfo.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, &item.set, item.setID);
                    VKRNDR->CMDRecordBindIndexedMesh(framecntx.CMDBuffer, item.mesh);
                    VKRNDR->CMDRecordPushConstant(framecntx.CMDBuffer, item.pipeInfo.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, item.push);
                    VKRNDR->CMDRecordDrawIndexedMesh(framecntx.CMDBuffer, item.mesh);
                }
            }
           // IMGUI PROBLY WILL HAVE TO MOVE IF WE ARE DOING BLOOM
            Ecntx.VKImGUI->RecordCMDBuffer(framecntx.CMDBuffer);           
            VKRNDR->CMDEndRenderPass(framecntx.CMDBuffer);
        }
        // 4 Stop recording
            VKRNDR->CMDEndRecord(framecntx.CMDBuffer);


            //Temp for Testing idealy later on the main pass waits on the fragemnt stage for
            // depth shadows too
            auto lightCullingDoneSemaphore = Ecntx.lightSys->GetLightCullReadySingal();
            
            VkSemaphore waitSems[] = {
                 framecntx.waitSemaphores,       
                 lightCullingDoneSemaphore       
            };

            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  
                                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT          
            };
         
        // 5 submit
            VKRNDR->CMDSubmitGraphicsQueue(&framecntx.CMDBuffer, 1, framecntx.currentFrameFence, waitStages, waitSems, 2, &framecntx.signalSemaphores, 1);
        // 6 present
            VKRNDR->CMDPresent(framecntx.targetFrameIndex, &framecntx.signalSemaphores,1);

    }

    static DrawItem GetDrawItem(const Ref<CActor>& actor,const VulkanRenderer::FrameContext& cntx) {

        auto mat = actor->GetComponent<CMaterial>();
        auto mesh = actor->GetComponent<CMesh>();
      

        DrawItem item{};
        item.push.modelMatrix = actor->GetModelMatrix();;
        item.push.normalMatrix = MMath::transpose(MMath::inverse(item.push.modelMatrix));
        item.pipeInfo = mat->GetPipelineInfo();
        item.mesh = mesh->GetMesh();
        item.set = mat->GetDescriptorSet()[cntx.inFlightIndex];
        item.setID = mat->GetSetValue();
        return item;
    }
};

void VulkanRenderer::RenderECS(const EngineContext& Ecntx,const std::vector<Ref<Component>>& drawlist)
{
    VulkanRenderer::ECSRenderer::Render(this, Ecntx,drawlist);
}
