#include "CMaterial.h"
#include "CShader.h"

bool CMaterial::OnCreate()
{
    return false;
}

void CMaterial::OnDestroy()
{
}

PipelineInfo CMaterial::GetPipelineInfo()
{
    if (Ref<CShader> s = shader.lock()) {
       
        return s->GetPipelineInfo();
    }
    return PipelineInfo();
}
