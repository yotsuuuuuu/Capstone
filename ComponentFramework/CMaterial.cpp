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
        for (const auto& file : textureFilePaths) { // owned by the mat class
            auto texture = vkrender->Create2DTextureImage(file.c_str());
            textures.push_back(texture);
        }

        if (Ref<CShader> s = shader.lock()) {
        descriptionSet = s->AllocateDescriptorSet(textures);
        }
        else {
            return false;
        }


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
        for (auto& texture : textures) {
            vkrender->DestroySampler2D(texture);
        }
        textures.clear();
        descriptionSet.clear();
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
