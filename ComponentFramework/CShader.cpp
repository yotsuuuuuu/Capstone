#include "CShader.h"
#include "VulkanRenderer.h"
#include "Debug.h"
bool CShader::OnCreate()
{
    if(isCreated)
		return true;
    if (!render)
        return false;
    
    switch (render->getRendererType()) {
    case RendererType::VULKAN :{
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);

        desInfo.descriptorSetLayout = vkrender->CreateDescriptorSetLayout(layoutInfo);
        desInfo.descriptorPool = vkrender->CreateDescriptorPool(layoutInfo, poolSize);

        std::vector<VkDescriptorSetLayout> arrDescriptorlayouts ={vkrender->GetGlobalDescriptionSet().descriptorSetLayout,desInfo.descriptorSetLayout};
        
        PipeLineConfig config = p_config.value_or(vkrender->GetMainPassPipeLineConfig());
		std::string filepath = "Cretead Shader with vert: " + vertShaderFile + " frag: " + fragShaderFile;
        Debug::Trace(filepath, "CSHADER", __LINE__);
        std::string viewportstring = "Viewport size: {" + std::to_string(config.viewPortsize.width) + "x" + std::to_string(config.viewPortsize.height) + "}";
        Debug::Trace(viewportstring, "CSHADER", __LINE__);
        pipelineInfo = vkrender->CreateGraphicsPipeline(arrDescriptorlayouts, config, vertShaderFile,
            fragShaderFile, tesCShaderFile, tesEShaderFile, geomShaderFile);
      
        
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
        vkrender->DestroyDescriptorSet(desInfo);
        vkrender->DestroyPipeline(pipelineInfo);
        break;
    }
    }

    isCreated = false;
}

void CShader::SetPipeLineConfig(PipeLineConfig config)
{
    p_config = config;
}


std::vector<VkDescriptorSet> CShader::AllocateDescriptorSet(std::vector<Sampler2D> arrySampler)
{
    VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
    if (vkrender) {
        std::vector<DescriptorWriteInfo> write;
        for (size_t i = 0; i < layoutInfo.size(); i++) {
            vkrender->AddToDescrisptorLayoutWrite(write, layoutInfo[i].binding, layoutInfo[i].descriptorType, DescriptorWriteInfo::Destype::STATIC_SAMPLER,
                layoutInfo[i].stageFlags, layoutInfo[i].descriptorCount, { arrySampler[i] });            
        }
        std::vector<VkDescriptorSet> set = vkrender->AllocateDescriptorSets(desInfo.descriptorPool, desInfo.descriptorSetLayout);

        vkrender->WriteDescriptorSets(set, write);

        return set;
    }
    return  std::vector<VkDescriptorSet>();
}

// Asumming this is being called while device is idel
void CShader::RecreatePipeLine()
{
    if (!isCreated)
        return;
    if (!render)
        return;

    switch (render->getRendererType()) {
    case RendererType::VULKAN: {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
      
        vkrender->DestroyPipeline(pipelineInfo);

        std::vector<VkDescriptorSetLayout> arrDescriptorlayouts = { vkrender->GetGlobalDescriptionSet().descriptorSetLayout,desInfo.descriptorSetLayout };

        PipeLineConfig config = p_config.value_or(vkrender->GetMainPassPipeLineConfig());

        std::string filepath = "Cretead Shader with vert: " + vertShaderFile + " frag: " + fragShaderFile;
        Debug::Trace(filepath, "CSHADER", __LINE__);
        std::string viewportstring = "Viewport size: {" + std::to_string(config.viewPortsize.width) + "x" + std::to_string(config.viewPortsize.height) + "}";
        Debug::Trace(viewportstring, "CSHADER", __LINE__);
        pipelineInfo = vkrender->CreateGraphicsPipeline(arrDescriptorlayouts, config, vertShaderFile,
            fragShaderFile, tesCShaderFile, tesEShaderFile, geomShaderFile);
        break;
    }
    }

}
