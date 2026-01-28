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


struct VulkanRenderer::ECSRenderer {

    static void Render(VulkanRenderer* renderer, std::vector<Component> drawlist) {
        // 1 Get current render frame info
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
};