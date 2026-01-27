#include "CMaterial.h"
#include "CShader.h"
#include "VulkanRenderer.h"

/// <summary>
/// Loads Texture to the GPU, It dose not make the descriptor set
/// it is assummed it will be set with SetDescriptorSet call;
/// </summary>
bool CMaterial::OnCreate()
{
    if (isCreated)
        return true;
    if (!render)
        return false;
    switch (render->getRendererType()) {
    case RendererType::VULKAN: {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
        texture = vkrender->Create2DTextureImage(textureFile);   
        isCreated = true;
        return true;
        break;
    }
    }

    return false;
}
/// <summary>
/// Deloads texture form GPU . Assumes device is idel 
/// </summary>
void CMaterial::OnDestroy()
{
    if (!isCreated)
        return ;
    if (!render)
        return ;
    switch (render->getRendererType()) {
    case RendererType::VULKAN: {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
        vkrender->DestroySampler2D(texture);      
        break;
    }
    }
    isCreated = false;
}

PipelineInfo CMaterial::GetPipelineInfo()
{
    if (Ref<CShader> s = shader.lock()) {
       
        return s->GetPipelineInfo();
    }
    return PipelineInfo();
}
