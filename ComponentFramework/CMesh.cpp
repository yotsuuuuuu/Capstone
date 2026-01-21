#include "CMesh.h"
#include "Debug.h"
#include "VulkanRenderer.h"

bool CMesh::OnCreate()
{
    if( isCreated)
		return true;
 

    return false;
}

void CMesh::OnDestroy()
{
    if (!isCreated)
		return;
    
}
