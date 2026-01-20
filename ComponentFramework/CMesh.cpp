#include "CMesh.h"
#include "Debug.h"
#include "VulkanRenderer.h"

bool CMesh::OnCreate()
{
    if( isCreated)
		return true;
    if (!renderer)
        return false;

    switch (renderer->getRendererType()) {
    case RendererType::VULKAN: {
		// this loads the model to the GPU and returns the indexed vertex buffer
		VulkanRenderer* vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
        mesh = vRenderer->LoadModelIndexed(meshFile);
        isCreated = true;
		return isCreated;
    }break;
    default:
        Debug::Error("Unsupported renderer type in CMesh::OnCreate()", __FILE__, __LINE__);
		return false;
    }

    return false;
}

void CMesh::OnDestroy()
{
    if (!isCreated)
		return;
    if (!renderer)
		return;
    switch (renderer->getRendererType())
    {
    case RendererType::VULKAN:
    {
        VulkanRenderer* vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
        vRenderer->DestroyIndexedMesh(mesh);
        isCreated = false;
    }
    default:
		Debug::Error("Unsupported renderer type in CMesh::OnDestroy()", __FILE__, __LINE__);
        break;
    }
}
