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
	VkDeviceSize bufferSize = sizeof(TerrainVertex) * vertices.size();
	outBuffer.vertBufferLength = bufferSize; // resize outBuffer length

	// create staging buffer
	BufferMemory stagingBuffer;
	CreateBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.bufferID, stagingBuffer.bufferMemoryID);

	void* data;
	vkMapMemory(device, stagingBuffer.bufferMemoryID, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBuffer.bufferMemoryID);

	CreateBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		outBuffer.vertBufferID, outBuffer.vertBufferMemoryID);

	CopyBuffer(stagingBuffer.bufferID, outBuffer.vertBufferID, bufferSize);

	vkDestroyBuffer(device, stagingBuffer.bufferID, nullptr);
	vkFreeMemory(device, stagingBuffer.bufferMemoryID, nullptr);

}

void VulkanRenderer::CreateTerrainIndexBuffer(const std::vector<uint32_t>& indices, IndexedVertexBuffer& outBuffers) // DEPRECATAED
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();
	outBuffers.indexBufferLength = bufferSize; // resize outBuffer length

	// create staging buffer
	BufferMemory stagingBuffer{};

	CreateBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.bufferID, stagingBuffer.bufferMemoryID);

	void* data;
	vkMapMemory(device, stagingBuffer.bufferMemoryID, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBuffer.bufferMemoryID);

	CreateBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		outBuffers.indexBufferID, outBuffers.indexBufferMemoryID);

	CopyBuffer(stagingBuffer.bufferID, outBuffers.indexBufferID, bufferSize);

	vkDestroyBuffer(device, stagingBuffer.bufferID, nullptr);
	vkFreeMemory(device, stagingBuffer.bufferMemoryID, nullptr);
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