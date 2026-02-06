#include "VulkanRenderer.h"
#include "CActor.h"
#include "CCameraActor.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CMesh.h"
#include "CTransform.h"
#include <unordered_map>
#include "imgui.h"

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
void VulkanRenderer::CreateRenderPass(VkRenderPass& renderpass, std::vector<VkAttachmentDescription> colorAD, std::optional<VkAttachmentDescription> depthAD)
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
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
private:
    struct DrawItem{
        PipelineInfo pipeInfo;
        VkDescriptorSet set;
        uint32_t setID;
        IndexedVertexBuffer mesh;
        ModelMatrixPushConst push;
    };
public:
    static void Render(VulkanRenderer* VKRNDR, const std::vector<Ref<Component>>& drawlist) {
        //  Passes i want to do currently doing forward rendering
        //  Sky light shadow pass : the Main light source that affects the scene
        //  Normal forward pass
        //  Post process bloom pass
        //  ImGUI 
        ImGuiIO& io = ImGui::GetIO();
        VKRNDR->imGuiSystem->BeginFrame();
        ImGui::Begin("Fps", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("%.3f ms/frame (%.1f FPS) Deltatime: ", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        VKRNDR->imGuiSystem->TestUI();
        VKRNDR->imGuiSystem->EndFrame();
        // 1 Get current render frame info
        VulkanRenderer::FrameContext framecntx =  VKRNDR->GetCurrentFrameContext();
        VkPipelineLayout line;
        // 1.1 get draw items
        std::unordered_map<VkPipeline, std::vector<DrawItem>> DrawingBuckets;
        for (const auto& comp : drawlist) {
            Ref<CActor> a = std::dynamic_pointer_cast<CActor>(comp);
            auto mat = a->GetComponent<CMaterial>();
            auto mesh = a->GetComponent<CMesh>();
            // sohuld check for transform
            if (a && mat && mesh) {
                DrawItem item = GetDrawItem(a, framecntx);
        // 1.2 sort them into buckets
                DrawingBuckets[item.pipeInfo.pipeline].push_back(item);
                line = item.pipeInfo.pipelineLayout;
            }
        }
        // 2 Update UBOs for current frame ??? needs to be done
        // 3 Start recording
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
            VKRNDR->CMDBeginRecord(framecntx.CMDBuffer);
            VKRNDR->CMDBeginRenderPass(framecntx.CMDBuffer, renderinfo);
            //global discriptor bind
            auto globalset = VKRNDR->GetGlobalDescriptionSet();
            VKRNDR->CMDRecordDescriptorSet(framecntx.CMDBuffer, line, VK_PIPELINE_BIND_POINT_GRAPHICS, &globalset.descriptorSet[framecntx.targetFrameIndex]);
            for (const auto& pair : DrawingBuckets) {
                VKRNDR->CMDRecordBindPipeline(framecntx.CMDBuffer, pair.first, VK_PIPELINE_BIND_POINT_GRAPHICS);
                for (const auto& item : pair.second) {
                    VKRNDR->CMDRecordDescriptorSet(framecntx.CMDBuffer, item.pipeInfo.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, &item.set, item.setID);
                    VKRNDR->CMDRecordBindIndexedMesh(framecntx.CMDBuffer, item.mesh);
                    VKRNDR->CMDRecordPushConstant(framecntx.CMDBuffer, item.pipeInfo.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, item.push);
                    VKRNDR->CMDRecordDrawIndexedMesh(framecntx.CMDBuffer, item.mesh);
                }
            VKRNDR->imGuiSystem->RecordCMDBuffer(framecntx.CMDBuffer);
            VKRNDR->CMDEndRenderPass(framecntx.CMDBuffer);
            }
        // 4 Stop recording
            VKRNDR->CMDEndRecord(framecntx.CMDBuffer);

            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
         /*   printf("%d cmd\t%d framefence\t%d waitSemaphore\t%d singalSemaphore\n", (int)framecntx.CMDBuffer, 
                (int)framecntx.currentframeFence, (int)framecntx.waitSemaphores, (int)framecntx.signalSemaphores);*/
        // 5 submit
            VKRNDR->CMDSubmitGraphics(&framecntx.CMDBuffer, 1, framecntx.currentframeFence, waitStages, &framecntx.waitSemaphores, 1, &framecntx.signalSemaphores, 1);
        // 6 present
            VKRNDR->CMDPresent(framecntx.targetFrameIndex, &framecntx.signalSemaphores,1);
        }
    }
    static DrawItem GetDrawItem(const Ref<CActor>& actor,const VulkanRenderer::FrameContext& cntx) {

        auto mat = actor->GetComponent<CMaterial>();
        auto mesh = actor->GetComponent<CMesh>();
      

        DrawItem item{};
        item.push.modelMatrix = actor->GetModelMatrix();;
        item.push.normalMatrix = MMath::transpose(MMath::inverse(item.push.modelMatrix));
        item.pipeInfo = mat->GetPipelineInfo();
        item.mesh = mesh->GetMesh();
        item.set = mat->GetDescriptorSet()[cntx.targetFrameIndex];
        item.setID = mat->GetSetValue();
        return item;
    }
};

void VulkanRenderer::RenderECS(const std::vector<Ref<Component>>& drawlist)
{
    VulkanRenderer::ECSRenderer::Render(this, drawlist);
}
