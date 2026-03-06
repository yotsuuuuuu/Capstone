#include "VulkanRenderer.h"

void VulkanRenderer::CreateTerrainBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffers)
{
	//CreateTerrainVertexBuffer(vertices, outBuffers);
	//CreateTerrainIndexBuffer(indices, outBuffers);
	CreateVertexBuffer(outBuffers, vertices);
	CreateIndexBuffer(outBuffers, indices);
}

void VulkanRenderer::CreateTerrainVertexBuffer(const std::vector<Vertex>& vertices, IndexedVertexBuffer& outBuffer) // DEPRECATAED
{
	CreateVertexBuffer(outBuffer, vertices);
}

void VulkanRenderer::CreateTerrainIndexBuffer(const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffer) // DEPRECATAED
{
	CreateIndexBuffer(outBuffer, indices);

}

void VulkanRenderer::RenderTerrainChunk(IndexedVertexBuffer& terrainBuffers, const ModelMatrixPushConst& transform, PipelineInfo& pipelineInfo, DescriptorSetInfo descriptorSet)
{
	BindPipeline(pipelineInfo.pipeline);
	BindDescriptorSet(pipelineInfo.pipelineLayout, descriptorSet.descriptorSet, 0);
	BindMesh(terrainBuffers);
	SetPushConstant(pipelineInfo, transform.modelMatrix);
	DrawTerrain(terrainBuffers);
}

PipelineInfo VulkanRenderer::CreateTerrainPipeline(VkDescriptorSetLayout descriptorSetLayout)
{
	PipelineInfo terrainPipeline = CreateGraphicsPipeline(descriptorSetLayout,
		"shaders/terrain.vert.spv",
		"shaders/terrain.frag.spv");

	return terrainPipeline;
}

void VulkanRenderer::DrawTerrain(IndexedVertexBuffer chunk)
{
	// convert byte size to element count
	uint32_t indexCount = static_cast<uint32_t>(chunk.indexBufferLength / sizeof(uint32_t));
	for (const auto& commandBuffer : primaryCommandBuffer.commandBuffers){
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}

}


void VulkanRenderer::CMDRecordDrawTerrainIndex(const VkCommandBuffer& cmd, const IndexedVertexBuffer& mesh)
{
	uint32_t indexCount = static_cast<uint32_t>(mesh.indexBufferLength );// / sizeof(uint32_t)
	vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}

