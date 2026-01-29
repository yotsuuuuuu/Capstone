#include "VulkanRenderer.h"
#include "CActor.h"
#include "CCameraActor.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CTransform.h"


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
    context.CMDBuffer = primaryCommandBuffer.commandBuffers[imageIndex];
    context.currentframeFence = inFlightFences[currentFrame];
    context.waitSemaphores = imageAvailableSemaphores[currentFrame];
    context.signalSemaphores = renderFinishedSemaphores[currentFrame];
    context.currentFrameBuffer = swapChainFramebuffers[imageIndex];
    context.Renderpass = renderPass;
    context.extent = swapChainExtent;

    return context;
   
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
    vkCmdBindDescriptorSets(cmd, flag, layout, fristSet, count, DesSet, desOffset, nullptr);
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

void VulkanRenderer::CMDSubmitGraphics(VkCommandBuffer* cmds, uint32_t cmd_count, VkFence fence,
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
    struct DrawItem{
        PipelineInfo pipeInfo;
        VkDescriptorSet set;
        uint32_t setID;
        IndexedVertexBuffer mesh;
        ModelMatrixPushConst push;
    };

    static void Render(VulkanRenderer* VKRNDR, const std::vector<Ref<Component>>& drawlist) {
        VulkanRenderer::FrameContext framecntx =  VKRNDR->GetCurrentFrameContext();
        // 1 Get current render frame info
        // 1.1 get draw items
        // 1.2 sort them into buckets
        // 2 Update UBOs for current frame
        // 3 Start recording
        //  Passes i want to do currently doing forward rendering
        //  Sky light shadow pass : the Main light source that affects the scene
        //  Normal forward pass
        //  Post process bloom pass
        //  ImGUI 
        // 4 Stop recording
        // 5 submit
        // need to nail donw: is this all in one cmd buffer? or does it need ot be broken up?
        
    }
    static DrawItem GetDrawItem(const Ref<CActor>& actor,const VulkanRenderer::FrameContext& cntx) {

    }
};

void VulkanRenderer::RenderECS(const std::vector<Ref<Component>>& drawlist)
{
    VulkanRenderer::ECSRenderer::Render(this, drawlist);
}
