#include "TerrainRenderer.h"

void TerrainRenderer::drawChunk(VkCommandBuffer commandBuffer, const TerrainRenderData& mesh, const Chunk& chunk)
{

	VkDeviceSize offsets[] = { 0 }; // adjust as needed?

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	// descriptor sets would go here 
	// contains heightmap buffer info
	// and the chunk world position info / offset

	vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0); // draw call
}
