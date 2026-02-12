#include "CSkyBox.h"
#include "VulkanRenderer.h"
#include "CShader.h"
#include "CMesh.h"

bool CSkyBox::OnCreate()
{
    if (isCreated)
        return true;
    if (!renderer)
        return false;

    switch (renderer->getRendererType()) {
    case RendererType::VULKAN: {
        VulkanRenderer* VKR = static_cast<VulkanRenderer*>(renderer);
        //create mesh
        Ref<CMesh> m = std::make_shared<CMesh>(nullptr, renderer, "MESHFILE");
        if (!m->OnCreate())
            return false;
        Mesh = m;
        //create shader
        std::vector<SingleDescriptorSetLayoutInfo> layout;
        VKR->AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
        auto config = VKR->GetMainPassPipeLineConfig();
        config.depthTestEnable = VK_TRUE;
        config.depthWriteEnable = VK_FALSE;
        config.depthCompareOp = VK_COMPARE_OP_LESS;
        config.cullMode = VK_CULL_MODE_FRONT_BIT;
        Ref<CShader> s = std::make_shared<CShader>(nullptr, renderer, layout, "VERT", "FRAG", config);
        if (!s->OnCreate())
            return false;
        Shader = s;
        //create skybox mat
        CubeSampler = VKR->SkyBoxSampler(paths);
        //create set
        DesSet = s->AllocateDescriptorSet({ CubeSampler });

        isCreated = true;
        return true;
        break;
    }
    default:
        break;
    }
    

    return false;
}

void CSkyBox::OnDestroy()
{
    //Destroy mesh
    //Destroy mesh
    //Destroy shader
    //Destroy set
    //Destroy skybox mat
}
