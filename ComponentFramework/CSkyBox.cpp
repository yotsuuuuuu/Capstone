#include "CSkyBox.h"
#include "VulkanRenderer.h"
#include "CShader.h"
#include "CMesh.h"
#include "imgui.h"
#include "FmodController.h"

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
        Ref<CMesh> m = std::make_shared<CMesh>(nullptr, renderer, "./meshes/Cube.obj");
        if (!m->OnCreate())
            return false;
        Mesh = m;
        //create shader
        std::vector<SingleDescriptorSetLayoutInfo> layout;
        VKR->AddToDescriptorLayoutCollection(layout, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
        auto config = VKR->GetMainPassPipeLineConfig();
        config.depthTestEnable = VK_TRUE;
        config.depthWriteEnable = VK_FALSE;
        config.depthCompareOp = VK_COMPARE_OP_LESS;
        config.cullMode = VK_CULL_MODE_FRONT_BIT;
        Ref<CShader> s = std::make_shared<CShader>(nullptr, renderer, layout, "shaders/SkyBox.vert.spv", "shaders/SkyBox.frag.spv", config);
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
    if (!isCreated)
        return;
    if (!renderer)
        return;

    switch (renderer->getRendererType())
    {
    case RendererType::VULKAN: {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(renderer);

        //Destroy mesh
        Mesh->OnDestroy();
        //Destroy shader
        //Destroy set
        Shader->OnDestroy();
        //Destroy skybox mat
        vkrender->DestroySampler2D(CubeSampler);
        DesSet.clear();

        isCreated = false;

        break;
    }
    default:

        break;
    }
}

IndexedVertexBuffer CSkyBox::GetMesh()
{
    return   std::dynamic_pointer_cast<CMesh>(Mesh)->GetMesh();
}

void CSkyBox::RecreatePipeline()
{
    if (!isCreated)
        return;
    if (!renderer)
        return;
    VulkanRenderer* VKR = static_cast<VulkanRenderer*>(renderer);
    auto config = VKR->GetMainPassPipeLineConfig();
    config.depthTestEnable = VK_TRUE;
    config.depthWriteEnable = VK_FALSE;
    config.depthCompareOp = VK_COMPARE_OP_LESS;
    config.cullMode = VK_CULL_MODE_FRONT_BIT;
    Ref<CShader> s = std::dynamic_pointer_cast<CShader>(Shader);
    s->SetPipeLineConfig(config);
    s->RecreatePipeLine();
}

PipelineInfo CSkyBox::GetPipeline()
{
    return std::dynamic_pointer_cast<CShader>(Shader)->GetPipelineInfo();
}

void CSkyBox::AudioReact(EngineContext& cntx)
{
    auto bands = cntx.fmodController->GetFrameAudioBand();
    float smoothing = 0.25f;
    m_smoothed1 += (bands.bass - m_smoothed1) * smoothing;
    m_smoothed2 += (bands.highBass - m_smoothed2) * smoothing;


    push.Bloomfactor = (m_smoothed1 + m_smoothed2) * 0.5f * 3.0f ;
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always); // visualy probly should be moved out of here.
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("Audio",nullptr, ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoBackground );
    ImGui::PlotLines("##Audio", (float*) & bands, 10, 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
    ImGui::End();
}

void CSkyBox::ImGui()
{

    ImGui::Begin("CSkyBox");
 
    ImGui::SliderFloat("Bloom Factor (0 -> 2)", &push.Bloomfactor, 0.0f, 2.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::ColorPicker4("RGB Bloom Tint", (float*) &push.ColorTint, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float);

    ImGui::End();
}
