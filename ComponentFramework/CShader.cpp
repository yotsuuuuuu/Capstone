#include "CShader.h"

bool CShader::OnCreate()
{
    if(isCreated)
		return true;

    return false;
}

void CShader::OnDestroy()
{
    if(!isCreated)
		return;
}
