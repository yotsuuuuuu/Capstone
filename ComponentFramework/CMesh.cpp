#include "CMesh.h"
#include "Debug.h"
#include "VulkanRenderer.h"

/// <summary>
/// Loads mesh data obj file to gpu memory.
/// 
/// </summary>
/// <returns>True : assested was loaded , False: failed to load</returns>
bool CMesh::OnCreate()
{
    if( isCreated)
		return true;
 
    if (!render)
        return false;
    switch (render->getRendererType())
    {
    case RendererType::VULKAN: {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
        mesh = vkrender->LoadModelIndexed(meshFile.c_str());
        isCreated = true;
        return true;
        break;
    }
    default:

        break;
    }

    return false;
}

/// <summary>
/// Deloads mesh form memory assumes gpu is idle when this is called.
/// </summary>
void CMesh::OnDestroy()
{
    if (!isCreated)
		return;
    if (!render)
        return;
    switch (render->getRendererType())
    {
    case RendererType::VULKAN: {
        VulkanRenderer* vkrender = static_cast<VulkanRenderer*>(render);
        vkrender->DestroyIndexedMesh(mesh);
        break;
    }
    default:
        break;
    }

    isCreated = false;
}
