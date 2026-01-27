#include "CShader.h"
#include "VulkanRenderer.h"
bool CShader::OnCreate()
{
    if(isCreated)
		return true;
    if (!render)
        return false;

    switch (render->getRendererType()) {
    case RendererType::VULKAN :{
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
        pipelineInfo = vkrender->CreateGraphicsPipeline(pipeLineLayout, vertShaderFile, fragShaderFile,
            tesCShaderFile, tesEShaderFile, geomShaderFile);
        isCreated = true;
        return true;
        break;
    }
    }

    return false;
}

void CShader::OnDestroy()
{
    if (!isCreated)
        return;
    if (!render)
        return;

    switch (render->getRendererType()) {
    case RendererType::VULKAN : {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
        vkrender->DestroyPipeline(pipelineInfo);
        break;
    }
    }

    isCreated = false;
}

void CShader::RecreatePipeLine()
{
}
