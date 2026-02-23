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

        desInfo.descriptorSetLayout = vkrender->CreateDescriptorSetLayout(layoutInfo);
        desInfo.descriptorPool = vkrender->CreateDescriptorPool(layoutInfo, poolSize);

        std::vector<VkDescriptorSetLayout> arrDescriptorlayouts ={vkrender->GetGlobalDescriptionSet().descriptorSetLayout,desInfo.descriptorSetLayout};
        
        PipeLineConfig config = p_config.value_or(vkrender->GetMainPassPipeLineConfig());
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

void CShader::RecreatePipeLine()
{
}
