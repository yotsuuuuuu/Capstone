#pragma once
#include <unordered_map>
#include "Vector.h"
#include "WorldConstants.h"
#include "CoreStructs.h"
#include "TerrainPreset.h"
#include "TerrainNoise.h"
#include "VulkanRenderer.h"
#include "Chunk.h"
#include "BaseGridMesh.h"
#include "CActor.h"

using namespace MATH;

class World {
private:
	Renderer* renderer;
	VulkanRenderer* vRenderer;
	TerrainNoise* terrainNoise;

	// chunks
	std::unique_ptr<BaseGridMesh> baseChunkMesh; // base mesh for chunks
	std::vector<std::unique_ptr<Chunk>> chunks; 
	std::unordered_map<Vec2, TerrainChunkData> chunkRenderData; // map chunk positions to their render data

	// vulkan
	PipelineInfo worldPipeline;
	DescriptorSetInfo worldDescriptorSet;

	//player
	//CActor* player;

	//texture
	Sampler2D terrainTexture;

public:

	World(Renderer* renderer_) : renderer(renderer_), terrainNoise(nullptr) {}
	~World();

	void Initialize(TerrainPreset* t_, std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_);
	void RenderWorld();

private:
	void GenerateAllChunks();
	void GenerateChunkHeightmap(Chunk* chunk);
	void BuildChunkMeshData(Chunk* chunk);
	void CalculateNormals(std::vector<TerrainVertex>& vertices);

	void CreateWorldPipeline(std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_);
	void CreateWorldDescriptorSet(std::vector<BufferMemory> cameraUBO, std::vector<BufferMemory> lightsUBO);
};